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
#include <dfm-base/base/device/devicealiasmanager.h>

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
        return parseCifsMountCrumb(url, mapGroup) || parseDevMountCrumb(url, mapGroup);
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
    auto devs = DevProxyMng->getAllBlockIds(GlobalServerDefines::kMounted | GlobalServerDefines::kSystem);
    for (const auto &dev : devs) {
        // 一些分区会有自己的别名，tab应该显示用户定义的别名
        auto info = InfoFactory::create<EntryFileInfo>(ComputerUtils::makeBlockDevUrl(dev));
        if (info && UniversalUtils::urlEquals(info->targetUrl(), url)) {
            *tabName = info->displayName();
            fmInfo() << "Set tab name to:" << *tabName << "for URL:" << url;
            return true;
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
        QRegularExpression udisksPrefixRegx(R"(^(/run/media/|/media/))");
        if (!path.contains(udisksPrefixRegx)) {
            //            fmInfo() << "not udisks mount path, ignore prehandle" << url;
            break;
        }

        fmInfo() << "Start checking if path should be writable:" << url;
        SyncFileInfo fileInfo(url);
        if (fileInfo.isAttributes(FileInfo::FileIsType::kIsWritable)
            && fileInfo.isAttributes(FileInfo::FileIsType::kIsExecutable)
            && fileInfo.isAttributes(FileInfo::FileIsType::kIsReadable)) {
            fmInfo() << "File has full permissions for current user, ignoring prehandle:" << url;
            break;
        }

        QString deviceID;
        if (!DevProxyMng->isMptOfDevice(path, deviceID)) {
            fmDebug() << "Path is not mountpoint of device, ignoring prehandle:" << url;
            break;
        }

        fmDebug() << "Found device ID for path:" << deviceID << "path:" << path;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QString devDesc = deviceID.split("/", QString::SkipEmptyParts).last();
#else
        QString devDesc = deviceID.split("/", Qt::SkipEmptyParts).last();
#endif
        if (devDesc.isEmpty() || !deviceID.startsWith("/org/freedesktop/UDisks")) {
            fmWarning() << "Cannot get device description or invalid device ID, ignoring prehandle. URL:" << url << "deviceID:" << deviceID;
            break;
        }

        QString ignoreFlagFilePath = QString("/tmp/dfm_%1_%2_ignore_request_permission_in_current_session")
                                             .arg(getuid())
                                             .arg(devDesc);
        QFile ignoreFlagFile(ignoreFlagFilePath);
        if (ignoreFlagFile.exists()) {
            fmInfo() << "User has ignored permission request before for device:" << devDesc;
            break;
        }

        auto info = DevProxyMng->queryBlockInfo(deviceID);
        if (!info.value(GlobalServerDefines::DeviceProperty::kHintSystem).toBool()) {
            fmDebug() << "Not system disk, ignoring prehandle for device:" << deviceID;
            break;
        }

        if (info.value(GlobalServerDefines::DeviceProperty::kFileSystem).toString().toLower() == "vfat") {
            fmInfo() << "VFAT filesystem detected, chmod operation not effective, skipping prehandle:" << deviceID;
            break;
        }

        if (info.value(GlobalServerDefines::DeviceProperty::kIsLoopDevice).toBool()) {
            fmInfo() << "Loop device detected, chmod operation not effective, skipping prehandle:" << deviceID;
            break;
        }

        // ask for user whether to chmod.
        const QString &deviceName = DeviceUtils::convertSuitableDisplayName(info);
        if (!askForConfirmChmod(deviceName)) {
            ignoreFlagFile.open(QIODevice::NewOnly);
            ignoreFlagFile.close();
            fmInfo() << "User dismissed chmod request for device:" << deviceName;
            break;
        }

        // do chmod
        fmInfo() << "Starting chmod operation for device:" << deviceName << "path:" << path;
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
    QString host, share, devName;
    bool ok = DeviceUtils::parseSmbInfo(devPath, host, share);
    if (ok) {
        QUrl smbUrl;
        smbUrl.setScheme("smb");
        smbUrl.setHost(host);
        const auto &alias = NPDeviceAliasManager::instance()->getAlias(smbUrl);
        if (!alias.isEmpty())
            host = alias;
        devName = ProtocolEntryFileEntity::tr("%1 on %2").arg(share, host);
    } else {
        devName = QFileInfo(devPath).fileName();
    }

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

bool ComputerEventReceiver::parseDevMountCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    // fix bug-326657
    QString filePath = url.path();
    if (!DevProxyMng->isFileOfExternalMounts(filePath))
        return false;

    const auto &deviceInfo = DevProxyMng->queryDeviceInfoByPath(filePath);
    auto id = deviceInfo.value(GlobalServerDefines::DeviceProperty::kId).toString();
    if (id.isEmpty())
        return false;

    QUrl devUrl = id.startsWith(kBlockDeviceIdPrefix)
            ? ComputerUtils::makeBlockDevUrl(id)
            : ComputerUtils::makeProtocolDevUrl(id);
    DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
    if (!info)
        return false;
    const auto &devPath = info->targetUrl().path();

    if (devPath.isEmpty() || devPath == "/")
        return false;

    QString iconName { info->fileIcon().name() };
    if (info->fileIcon().name().startsWith("media"))
        iconName = "media-optical-symbolic";
    else if (info->order() == AbstractEntryFileEntity::kOrderRemovableDisks)   // always display as USB icon for removable disks.
        iconName = "drive-removable-media-symbolic";
    else if (iconName == "android-device")
        iconName = "phone-symbolic";
    else if (iconName == "ios-device")
        iconName = "phone-apple-iphone-symbolic";
    else
        iconName += "-symbolic";

    QVariantMap rootNode { { "CrumbData_Key_Url", QUrl::fromLocalFile(devPath) },
                           { "CrumbData_Key_IconName", iconName },
                           { "CrumbData_Key_DisplayText", "" } };
    mapGroup->push_back(rootNode);

    QString currPrefix = devPath;
    auto subPathList = filePath.remove(devPath).split("/", Qt::SkipEmptyParts);
    while (subPathList.count() > 0) {
        QString dirName = subPathList.takeFirst();
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
    dlg.setIcon(QIcon::fromTheme("dde-file-manager"));
    dlg.addButton(tr("Cancel"));
    int confirmIdx = dlg.addButton(tr("Enable Now"), true, DDialog::ButtonRecommend);

    return dlg.exec() == confirmIdx;
}

ComputerEventReceiver::ComputerEventReceiver(QObject *parent)
    : QObject(parent)
{
}
