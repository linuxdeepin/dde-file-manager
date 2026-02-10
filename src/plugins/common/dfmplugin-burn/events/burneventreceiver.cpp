// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burneventreceiver.h"
#include "dialogs/burnoptdialog.h"
#include "dialogs/dumpisooptdialog.h"
#include "utils/burnhelper.h"
#include "utils/burnjobmanager.h"
#include "events/burneventcaller.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-framework/event/event.h>

#include <DDialog>
#include <QtConcurrent>
#include <QDir>

using namespace dfmplugin_burn;
using namespace GlobalServerDefines;
DFMBASE_USE_NAMESPACE

BurnEventReceiver::BurnEventReceiver(QObject *parent)
    : QObject(parent)
{
}

BurnEventReceiver *BurnEventReceiver::instance()
{
    static BurnEventReceiver receiver;
    return &receiver;
}

void BurnEventReceiver::handleShowBurnDlg(const QString &dev, bool isSupportedUDF, QWidget *parent)
{
    QString devId { DeviceUtils::getBlockDeviceId(dev) };
    auto &&map = DevProxyMng->queryBlockInfo(devId);

    QString defaultDiscName { qvariant_cast<QString>(map[DeviceProperty::kIdLabel]) };
    QStringList speed { qvariant_cast<QStringList>(map[DeviceProperty::kOpticalWriteSpeed]) };
    QString fileSystem { qvariant_cast<QString>(map[DeviceProperty::kFileSystem]) };
    bool disableISOOpts { isSupportedUDF };

    if (fileSystem.isEmpty())
        disableISOOpts = false;

    QScopedPointer<BurnOptDialog> dlg { new BurnOptDialog(dev, parent) };
    dlg->setDefaultVolName(defaultDiscName);
    dlg->setUDFSupported(isSupportedUDF, disableISOOpts);
    dlg->setWriteSpeedInfo(speed);
    dlg->exec();
}

void BurnEventReceiver::handleShowDumpISODlg(const QString &devId)
{
    QScopedPointer<DumpISOOptDialog> dlg { new DumpISOOptDialog(devId, qApp->activeWindow()) };
    dlg->exec();
}

void BurnEventReceiver::handleErase(const QString &dev)
{
    DWIDGET_USE_NAMESPACE

    if (BurnHelper::showOpticalBlankConfirmationDialog() == DDialog::Accepted)
        BurnJobManager::instance()->startEraseDisc(dev);
}

void BurnEventReceiver::handlePasteTo(const QList<QUrl> &urls, const QUrl &dest, bool isCopy)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    QString dev { BurnHelper::burnDestDevice(dest) };
    if (urls.size() == 1) {
        QDir destDir { BurnHelper::localStagingFile(dev).path() };
        destDir.setFilter(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
        QString devId { DeviceUtils::getBlockDeviceId(dev) };
        auto &&map = DevProxyMng->queryBlockInfo(devId);
        bool isBlank { DeviceUtils::isBlankOpticalDisc(devId) };

        auto fi { InfoFactory::create<FileInfo>(urls.front()) };
        static const QSet<QString> imageTypes { Global::Mime::kTypeCdImage, Global::Mime::kTypeISO9660Image };

        if (isBlank && fi && imageTypes.contains(fi->nameOf(NameInfoType::kMimeTypeName)) && destDir.count() == 0) {
            int r { BurnHelper::showOpticalImageOpSelectionDialog() };
            if (r == 1) {
                qint64 srcSize { fi->size() };
                qint64 avil { qvariant_cast<qint64>(map[DeviceProperty::kSizeFree]) };
                if (avil == 0 || srcSize > avil) {
                    DialogManagerInstance->showMessageDialog(tr("Unable to burn. Not enough free space on the target disk."));
                } else {
                    QScopedPointer<BurnOptDialog> dlg { new BurnOptDialog(dev, qApp->activeWindow()) };
                    dlg->setISOImage(urls.front());
                    dlg->exec();
                }
                return;
            }

            if (r == 0 || r == -1)
                return;
        }
    }

    QUrl tmpDest { BurnHelper::localStagingFile(dest) };
    QFileInfo fileInfo(tmpDest.path());
    if (fileInfo.isFile())
        tmpDest = UrlRoute::urlParent(tmpDest);
    QDir().mkpath(tmpDest.toLocalFile());

    BurnEventCaller::sendPasteFiles(urls, tmpDest, isCopy);
}

void BurnEventReceiver::handleCopyFilesResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(ok)

    if (srcUrls.isEmpty() || destUrls.isEmpty())
        return;

    QList<QUrl> discUrls;

    // change files permission (from disc to native)
    if (srcUrls.size() == destUrls.size()) {
        int index = 0;
        for (auto &&url : srcUrls) {
            if (DevProxyMng->isFileFromOptical(url.toLocalFile())) {
                QUrl destUrl { destUrls.at(index) };
                discUrls.append(destUrl);

                fmInfo() << "Add write permission for " << destUrl;
                auto permissions = (QFileInfo(destUrl.toLocalFile()).permissions() | QFileDevice::WriteUser
                                    | QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ReadOther);
                LocalFileHandler().setPermissionsRecursive(destUrl, permissions);
            }
            ++index;
        }
    }

    // write audit log (from disc to native)
    if (!discUrls.isEmpty() && discUrls.size() == srcUrls.size())
        BurnJobManager::instance()->startAuditLogForCopyFromDisc(srcUrls, discUrls);

    // staging map (from native to disc)
    if (BurnHelper::burnIsOnLocalStaging(destUrls.at(0)))
        BurnHelper::mapStagingFilesPath(srcUrls, destUrls);

    // packet writing
    QUrl directoryUrl { destUrls.at(0).adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) };
    QString dev { DeviceUtils::getMountInfo(directoryUrl.toLocalFile(), false) };
    if (!dev.isEmpty() && dev.startsWith("/dev/sr") && DeviceUtils::isPWUserspaceOpticalDiscDev(dev))
        BurnJobManager::instance()->startPutFilesToDisc(dev, destUrls);
}

void BurnEventReceiver::handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(ok)

    if (srcUrls.isEmpty() || destUrls.isEmpty())
        return;

    if (!destUrls.at(0).isLocalFile())
        return;

    // packet writing
    QUrl directoryUrl { destUrls.at(0).adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) };
    QString dev { DeviceUtils::getMountInfo(directoryUrl.toLocalFile(), false) };
    if (!dev.isEmpty() && dev.startsWith("/dev/sr") && DeviceUtils::isPWUserspaceOpticalDiscDev(dev))
        BurnJobManager::instance()->startPutFilesToDisc(dev, destUrls);
}

void BurnEventReceiver::handleFileRemoveResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(ok)

    if (srcUrls.isEmpty())
        return;

    if (!srcUrls.at(0).isLocalFile())
        return;

    // packet writing
    QUrl directoryUrl { srcUrls.at(0).adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) };
    QString dev { DeviceUtils::getMountInfo(directoryUrl.toLocalFile(), false) };
    if (!dev.isEmpty() && dev.startsWith("/dev/sr") && DeviceUtils::isPWUserspaceOpticalDiscDev(dev))
        BurnJobManager::instance()->startRemoveFilesFromDisc(dev, srcUrls);
}

void BurnEventReceiver::handleFileRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(winId);

    if (!ok || renamedUrls.isEmpty())
        return;

    for (const auto &src : renamedUrls.keys()) {
        if (!src.isLocalFile())
            return;
        QUrl srcDirUrl { src.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) };
        QString dev { DeviceUtils::getMountInfo(srcDirUrl.toLocalFile(), false) };
        if (dev.isEmpty() || !dev.startsWith("/dev/sr"))
            return;

        const auto &dest { renamedUrls.value(src) };
        QUrl destDirUrl { dest.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) };
        if ((srcDirUrl == destDirUrl) && DeviceUtils::isPWUserspaceOpticalDiscDev(dev))
            BurnJobManager::instance()->startRenameFileFromDisc(dev, src, dest);
    }
}

void BurnEventReceiver::handleMountImage(quint64 winId, const QUrl &isoUrl)
{
    fmInfo() << "Mount image:" << isoUrl;
    QString archiveuri;
    auto info { InfoFactory::create<FileInfo>(isoUrl) };
    if (info && info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)) {
        archiveuri = "archive://" + QString(QUrl::toPercentEncoding(info->urlOf(UrlInfoType::kRedirectedFileUrl).toString()));
        fmInfo() << "Mount image redirect the url to:" << info->urlOf(UrlInfoType::kRedirectedFileUrl);
    } else {
        archiveuri = "archive://" + QString(QUrl::toPercentEncoding(isoUrl.toString()));
    }

    QStringList args;
    args << "mount" << archiveuri;
    QProcess *gioproc = new QProcess;
    gioproc->start("gio", args);
    connect(gioproc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int ret) {
        if (ret) {
            DialogManagerInstance->showErrorDialog(tr("Mount error: unsupported image format"), QString());
        } else {
            QString doubleEncodedUri { QUrl::toPercentEncoding(isoUrl.toEncoded()) };
            doubleEncodedUri = QUrl::toPercentEncoding(doubleEncodedUri);
            QString id = QString("archive://%1/").arg(doubleEncodedUri);
            auto info = DevProxyMng->queryProtocolInfo(id);
            if (info.size() != 0) {
                QUrl mpt = QUrl::fromLocalFile(info.value(DeviceProperty::kMountPoint).toString());
                dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, mpt);
            } else {
                fmWarning() << "archive mount: cannot query mount info: " << doubleEncodedUri;
            }
        }
        gioproc->deleteLater();
    });
}
