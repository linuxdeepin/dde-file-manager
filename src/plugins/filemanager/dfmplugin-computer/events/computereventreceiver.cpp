// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computereventreceiver.h"
#include "computereventcaller.h"
#include "controller/computercontroller.h"
#include "utils/computerutils.h"
#include "fileentity/protocolentryfileentity.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <DDialog>

#include <QDebug>
#include <QDBusInterface>
#include <QApplication>

#include <unistd.h>
#include <sys/stat.h>

using namespace dfmplugin_computer;
DFMBASE_USE_NAMESPACE

ComputerEventReceiver *ComputerEventReceiver::instance()
{
    static ComputerEventReceiver ins;
    return &ins;
}

void ComputerEventReceiver::handleItemEject(const QUrl &url)
{
    ComputerControllerInstance->actEject(url);
}

bool ComputerEventReceiver::handleSepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    if (url.scheme() == ComputerUtils::scheme()) {
        QVariantMap map;
        map["CrumbData_Key_Url"] = url;
        map["CrumbData_Key_DisplayText"] = tr("Computer");
        map["CrumbData_Key_IconName"] = ComputerUtils::icon().name();
        mapGroup->push_back(map);
        return true;
    } else if (url.scheme() == Global::Scheme::kFile) {
        return parseCifsMountCrumb(url, mapGroup);
    }

    return false;
}

bool ComputerEventReceiver::handleSortItem(const QString &group, const QString &subGroup, const QUrl &a, const QUrl &b)
{
    if (!(group == "Group_Device" || group == "Group_Network"))
        return false;

    if (!(subGroup == Global::Scheme::kComputer || subGroup == Global::Scheme::kSmb || subGroup == Global::Scheme::kFtp))
        return false;

    return ComputerUtils::sortItem(a, b);
}

bool ComputerEventReceiver::handleSetTabName(const QUrl &url, QString *tabName)
{
    // if the url is the mount point of inner disk, set it to alias if alias if not empty.
    auto devs = DevProxyMng->getAllBlockIds(GlobalServerDefines::kMounted | GlobalServerDefines::kSystem);
    for (const auto &dev : devs) {
        auto devInfo = DevProxyMng->queryBlockInfo(dev);
        auto mpt = QUrl::fromLocalFile(devInfo.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString());
        if (UniversalUtils::urlEquals(mpt, url)) {
            auto info = InfoFactory::create<EntryFileInfo>(ComputerUtils::makeBlockDevUrl(dev));
            if (info) {
                *tabName = info->displayName();
                return true;
            }
        }
    }
    return false;
}

void ComputerEventReceiver::setContextMenuEnable(bool enable)
{
    ComputerUtils::contextMenuEnabled = enable;
}

void ComputerEventReceiver::dirAccessPrehandler(quint64, const QUrl &url, std::function<void()> after)
{
    do {
        const QString &path = url.path();
        if (url.scheme() != Global::Scheme::kFile) {
            //            fmInfo() << "not file scheme, ignore prehandle" << url;
            break;
        }
        // only handle mounts by udisks
        if (!path.startsWith("/media/")) {
            //            fmInfo() << "not udisks mount path, ignore prehandle" << url;
            break;
        }

        fmInfo() << "start checking if path should be writable" << url;
        SyncFileInfo fileInfo(url);
        if (fileInfo.isAttributes(FileInfo::FileIsType::kIsWritable)
            && fileInfo.isAttributes(FileInfo::FileIsType::kIsExecutable)
            && fileInfo.isAttributes(FileInfo::FileIsType::kIsReadable)) {
            fmInfo() << "file for current user is full permission, ignore prehandle" << url;
            break;
        }

        QString deviceID;
        if (!DevProxyMng->isMptOfDevice(path, deviceID)) {
            fmInfo() << "path is not mountpoint of device, ignore prehandle" << url;
            break;
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QString devDesc = deviceID.split("/", QString::SkipEmptyParts).last();
#else
        QString devDesc = deviceID.split("/", Qt::SkipEmptyParts).last();
#endif
        if (devDesc.isEmpty() || !deviceID.startsWith("/org/freedesktop/UDisks")) {
            fmInfo() << "cannot get the device description, ignore prehandle" << url << deviceID;
            break;
        }

        QString ignoreFlagFilePath = QString("/tmp/dfm_%1_%2_ignore_request_permission_in_current_session")
                                             .arg(getuid())
                                             .arg(devDesc);
        QFile ignoreFlagFile(ignoreFlagFilePath);
        if (ignoreFlagFile.exists()) {
            fmInfo() << "user has ignored prehandle before" << url << deviceID;
            break;
        }

        auto info = DevProxyMng->queryBlockInfo(deviceID);
        if (!info.value(GlobalServerDefines::DeviceProperty::kHintSystem).toBool()) {
            fmInfo() << "not system disk, ignore prehandle" << url << deviceID;
            break;
        }

        if (info.value(GlobalServerDefines::DeviceProperty::kFileSystem).toString().toLower() == "vfat") {
            fmInfo() << "chmod for vfat is useless, give up prehandle" << url << deviceID;
            break;
        }

        if (info.value(GlobalServerDefines::DeviceProperty::kIsLoopDevice).toBool()) {
            fmInfo() << "chmod for readonly loop device doesn't work, give up prehandle" << url << deviceID;
            break;
        }

        // ask for user whether to chmod.
        const QString &deviceName = DeviceUtils::convertSuitableDisplayName(info);
        if (!askForConfirmChmod(deviceName)) {
            ignoreFlagFile.open(QIODevice::NewOnly);
            ignoreFlagFile.close();
            fmInfo() << "user dismissed for chmod" << url << deviceID;
            break;
        }

        // do chmod
        fmInfo() << "start invoking Chmod" << url << deviceID;
        QDBusInterface daemonIface("org.deepin.Filemanager.AccessControlManager",
                                   "/org/deepin/Filemanager/AccessControlManager",
                                   "org.deepin.Filemanager.AccessControlManager",
                                   QDBusConnection::systemBus());
        auto ret = daemonIface.callWithArgumentList(QDBus::BlockWithGui, "Chmod", { path, static_cast<uint>(ACCESSPERMS) });
        fmInfo() << "Chmod finished for" << url << deviceID << ret;
    } while (0);

    if (after)
        after();
}

bool ComputerEventReceiver::parseCifsMountCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    const QString &filePath = url.path();
    static const QRegularExpression kCifsPrefix(R"(^/(?:run/)?media/.*/smbmounts)");
    auto match = kCifsPrefix.match(filePath);
    if (!match.hasMatch())
        return false;

    QVariantMap rootNode { { "CrumbData_Key_Url", QUrl::fromLocalFile(match.captured()) },
                           { "CrumbData_Key_IconName", "drive-harddisk-symbolic" },
                           { "CrumbData_Key_DisplayText", "" } };
    mapGroup->push_back(rootNode);

    static const QRegularExpression kCifsDevId(R"(^/(?:run/)?media/.*/smbmounts/[^/]*)");
    match = kCifsDevId.match(filePath);
    if (!match.hasMatch())
        return true;

    const QString &devPath = match.captured();
    QString host, share;
    bool ok = DeviceUtils::parseSmbInfo(devPath, host, share);
    QString devName = ok ? ProtocolEntryFileEntity::tr("%1 on %2").arg(share).arg(host)
                         : QFileInfo(devPath).fileName();

    QVariantMap devNode { { "CrumbData_Key_Url", QUrl::fromLocalFile(match.captured()) },
                          { "CrumbData_Key_DisplayText", devName } };
    mapGroup->push_back(devNode);

    // parse dir nodes.
    auto path = filePath;
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QStringList subPaths = path.remove(kCifsDevId).split("/", QString::SkipEmptyParts);
#else
    QStringList subPaths = path.remove(kCifsDevId).split("/", Qt::SkipEmptyParts);
#endif

    QString currPrefix = devPath;
    while (subPaths.count() > 0) {
        QString dirName = subPaths.takeFirst();
        currPrefix = currPrefix + "/" + dirName;
        QVariantMap dirNode { { "CrumbData_Key_Url", QUrl::fromLocalFile(currPrefix) },
                              { "CrumbData_Key_DisplayText", dirName } };
        mapGroup->push_back(dirNode);
    }
    return true;
}

bool ComputerEventReceiver::askForConfirmChmod(const QString &devName)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    using namespace Dtk::Widget;
    DDialog dlg(tr("%1 is read-only. Do you want to enable read and write permissions for it?").arg(devName),
                tr("Once enabled, read/write permission will be granted permanently"), qApp->activeWindow());
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(tr("Cancel"));
    int confirmIdx = dlg.addButton(tr("Enable Now"), true, DDialog::ButtonRecommend);

    return dlg.exec() == confirmIdx;
}

ComputerEventReceiver::ComputerEventReceiver(QObject *parent)
    : QObject(parent)
{
}
