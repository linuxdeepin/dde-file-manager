/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "burneventreceiver.h"
#include "dialogs/burnoptdialog.h"
#include "dialogs/dumpisooptdialog.h"
#include "utils/burnhelper.h"
#include "utils/burnjobmanager.h"
#include "events/burneventcaller.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <DDialog>
#include <QtConcurrent>
#include <QDir>

using namespace dfmplugin_burn;
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

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
    QScopedPointer<DumpISOOptDialog> dlg { new DumpISOOptDialog(devId) };
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
        bool isBlank { map[DeviceProperty::kOpticalBlank].toBool() };
        auto fi { InfoFactory::create<AbstractFileInfo>(urls.front()) };
        static const QSet<QString> imageTypes { Global::Mime::kTypeCdImage, Global::Mime::kTypeISO9660Image };

        if (isBlank && fi && imageTypes.contains(fi->nameOf(NameInfoType::kMimeTypeName)) && destDir.count() == 0) {
            int r { BurnHelper::showOpticalImageOpSelectionDialog() };
            if (r == 1) {
                qint64 srcSize { fi->size() };
                qint64 avil { qvariant_cast<qint64>(map[DeviceProperty::kSizeFree]) };
                if (avil == 0 || srcSize > avil) {
                    DialogManagerInstance->showMessageDialog(DialogManager::kMsgWarn,
                                                             tr("Unable to burn. Not enough free space on the target disk."));
                } else {
                    QScopedPointer<BurnOptDialog> dlg { new BurnOptDialog(dev) };
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

    if (srcUrls.isEmpty())
        return;

    QList<QUrl> discUrls;

    // change files permission (from disc to native)
    if (srcUrls.size() == destUrls.size()) {
        int index = 0;
        for (auto &&url : srcUrls) {
            if (DevProxyMng->isFileFromOptical(url.toLocalFile())) {
                QUrl destUrl { destUrls.at(index) };
                discUrls.append(destUrl);

                qInfo() << "Add write permission for " << destUrl;
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
    if (!destUrls.isEmpty() && BurnHelper::burnIsOnLocalStaging(destUrls.at(0)))
        BurnHelper::mapStagingFilesPath(srcUrls, destUrls);
}

void BurnEventReceiver::handleMountImage(quint64 winId, const QUrl &isoUrl)
{
    qInfo() << "Mount image:" << isoUrl;
    QString archiveuri;
    auto info { InfoFactory::create<AbstractFileInfo>(isoUrl) };
    if (info && info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)) {
        archiveuri = "archive://" + QString(QUrl::toPercentEncoding(info->urlOf(UrlInfoType::kRedirectedFileUrl).toString()));
        qInfo() << "Mount image redirect the url to:" << info->urlOf(UrlInfoType::kRedirectedFileUrl);
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
                qWarning() << "archive mount: cannot query mount info: " << doubleEncodedUri;
            }
        }
        gioproc->deleteLater();
    });
}
