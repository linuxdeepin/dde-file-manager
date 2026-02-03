// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_server_defines.h"
#include "dockitemdatamanager.h"
#include "utils/dockutils.h"

#include <dtkgui_global.h>
#include <dtkwidget_global.h>
#include <DDesktopServices>
#include <QTimer>
#include <QMutex>
#include <QSet>
#include <fstab.h>
#include <sys/stat.h>

Q_DECLARE_LOGGING_CATEGORY(logAppDock)

DGUI_BEGIN_NAMESPACE
DGUI_END_NAMESPACE

static constexpr char kDeviceDaemonName[] { "org.deepin.Filemanager.Daemon" };
static constexpr char kDevMngPath[] { "/org/deepin/Filemanager/Daemon/DeviceManager" };
static const bool kDisplay = true;
static const bool kIgnore = false;

DockItemDataManager *DockItemDataManager::instance()
{
    static DockItemDataManager ins;
    return &ins;
}

DockItemDataManager::DockItemDataManager(QObject *parent)
    : QObject { parent }
{
    devMng.reset(new DeviceManager(kDeviceDaemonName,
                                   kDevMngPath,
                                   QDBusConnection::sessionBus(),
                                   this));
    connectDeviceManger();
    watchService();
}

void DockItemDataManager::onBlockMounted(const QString &id)
{
    QVariantMap data = devMng->QueryBlockDeviceInfo(id, true);
    if (blockDeviceFilter(data)) {
        auto item = buildBlockItem(data);
        blocks.insert(id, item);
        Q_EMIT mountAdded(item);
        updateDockVisible();
    }
}

void DockItemDataManager::onBlockUnmounted(const QString &id)
{
    if (!blocks.contains(id)) {
        qCDebug(logAppDock) << "Block device not in blocks list, ignoring unmount:" << id;
        return;
    }

    blocks.remove(id);
    Q_EMIT mountRemoved(id);
    updateDockVisible();
}

void DockItemDataManager::onBlockPropertyChanged(const QString &id, const QString &property, const QDBusVariant &value)
{
    if (property == "HintIgnore") {
        bool val = value.variant().toBool();
        if (val)
            onBlockUnmounted(id);
        else
            onBlockMounted(id);
    }

    // 光盘被物理弹出
    if (id.contains(QRegularExpression("/sr[0-9]*$"))
        && property == GlobalServerDefines::DeviceProperty::kMediaAvailable
        && !value.variant().toBool()) {
        qCInfo(logAppDock) << "Optical disc physically ejected:" << id;
        onBlockUnmounted(id);
    }
}

void DockItemDataManager::onProtocolMounted(const QString &id)
{
    QVariantMap data = devMng->QueryProtocolDeviceInfo(id, true);
    if (protoDeviceFilter(data)) {
        auto item = buildProtocolItem(data);
        protocols.insert(id, item);
        Q_EMIT mountAdded(item);
        updateDockVisible();
    }
}

void DockItemDataManager::onProtocolUnmounted(const QString &id)
{
    protocols.remove(id);
    Q_EMIT mountRemoved(id);
    updateDockVisible();
}

bool DockItemDataManager::blockDeviceFilter(const QVariantMap &data)
{
    if (data.value(GlobalServerDefines::DeviceProperty::kHintIgnore).toBool())
        return kIgnore;
    if (data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString().isEmpty())
        return kIgnore;

    // Ignore devices with mount points in fstab (system mounted devices)
    QString mountPoint = data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
    if (isMountPointInFstab(mountPoint)) {
        qCDebug(logAppDock) << "Ignoring device with fstab mount point:" << mountPoint;
        return kIgnore;
    }

    if (data.value(GlobalServerDefines::DeviceProperty::kOpticalDrive).toBool())
        return kDisplay;

    // partitions should not be displayed in dock if they are siblings of root partition.
    if (isRootDrive(data.value(GlobalServerDefines::DeviceProperty::kDrive).toString()))
        return kIgnore;

    QString backingDevID = data.value(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice).toString();
    if (backingDevID != "/") {
        auto reply = devMng->QueryBlockDeviceInfo(backingDevID, false);
        reply.waitForFinished();
        auto backingData = reply.value();
        if (backingData.value(GlobalServerDefines::DeviceProperty::kConnectionBus).toString() != "usb")
            return kIgnore;
        return kDisplay;
    }
    if (data.value(GlobalServerDefines::DeviceProperty::kConnectionBus).toString() != "usb")
        return kIgnore;

    return kDisplay;
}

bool DockItemDataManager::protoDeviceFilter(const QVariantMap &data)
{
    const QString deviceId = data.value(GlobalServerDefines::DeviceProperty::kId).toString();
    const QString mountPoint = data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();

    // dlnfs mounts will be captured by gvfs, ignore them.
    if (device_utils::isDlnfsMount(mountPoint)) {
        qCDebug(logAppDock) << "DLNFS mount detected, ignoring:" << deviceId;
        return kIgnore;
    }

    return kDisplay;
}

bool DockItemDataManager::isRootDrive(const QString &drivePath)
{
    static std::once_flag flg;
    static QString rootDrive;
    std::call_once(flg, [this] {
        const QString &rootDev = device_utils::queryDevice("/");
        QString rootBlkPath = QString("/org/freedesktop/UDisks2/block_devices/") + rootDev.mid(5);
        qCInfo(logAppDock) << "the root object path is:" << rootBlkPath;
        const auto &rootDevData = devMng->QueryBlockDeviceInfo(rootBlkPath, false);
        rootDrive = rootDevData.value().value(GlobalServerDefines::DeviceProperty::kDrive).toString();
    });
    return rootDrive == drivePath;
}

bool DockItemDataManager::isMountPointInFstab(const QString &mountPoint)
{
    if (mountPoint.isEmpty())
        return false;

    static QMutex mutex;
    static QSet<QString> fstabMountPoints;
    static quint32 lastModify = 0;

    struct stat statInfo;
    if (stat("/etc/fstab", &statInfo) != 0) {
        qCWarning(logAppDock) << "Failed to stat /etc/fstab";
        return false;
    }

    QMutexLocker locker(&mutex);
    if (lastModify != static_cast<quint32>(statInfo.st_mtime)) {
        lastModify = static_cast<quint32>(statInfo.st_mtime);
        fstabMountPoints.clear();

        struct fstab *fs;
        setfsent();
        while ((fs = getfsent()) != nullptr) {
            fstabMountPoints.insert(QString(fs->fs_file));
        }
        endfsent();

        qCDebug(logAppDock) << "Updated fstab mount points, count:" << fstabMountPoints.size();
    }

    return fstabMountPoints.contains(mountPoint);
}

void DockItemDataManager::playSoundOnDevPlugInOut(bool in)
{
    DGUI_USE_NAMESPACE
    DWIDGET_USE_NAMESPACE
    DDesktopServices::playSystemSoundEffect(in ? DDesktopServices::SSE_DeviceAdded
                                               : DDesktopServices::SSE_DeviceRemoved);
    if (!in)
        notify(tr("The device has been safely removed"), "");
}

void DockItemDataManager::sendNotification(const QString &id, const QString &operation)
{
    qCInfo(logAppDock) << "eject failed: " << id << operation;
    if (!blocks.contains(id) && !protocols.contains(id)) {
        qCWarning(logAppDock) << "Device not found in blocks or protocols for notification:" << id;
        return;
    }

    QString devName = blocks.contains(id)
            ? blocks.value(id).displayName
            : protocols.value(id).displayName;
    qCInfo(logAppDock) << "device" << devName << operation << "failed";

    QMap<QString, QString> texts {
        { GlobalServerDefines::DeviceBusyAction::kEject, tr("eject") },
        { GlobalServerDefines::DeviceBusyAction::kUnmount, tr("unmount") },
        { GlobalServerDefines::DeviceBusyAction::kPowerOff, tr("remove") },
    };
    QString title = tr("Operation failed");
    QString msg = tr("Device (%1) is busy, cannot %2 now.")
                          .arg(devName)
                          .arg(texts.value(operation, tr("remove")));

    notify(title, msg);
}

void DockItemDataManager::onServiceRegistered()
{
    devMng.reset(new DeviceManager(kDeviceDaemonName,
                                   kDevMngPath,
                                   QDBusConnection::sessionBus(),
                                   this));
    connectDeviceManger();
    initialize();
}

void DockItemDataManager::onServiceUnregistered()
{
    QStringList blkIDs = blocks.keys();
    for (auto id : blkIDs)
        Q_EMIT mountRemoved(id);
    QStringList protoIDs = protocols.keys();
    for (auto id : protoIDs)
        Q_EMIT mountRemoved(id);

    Q_EMIT requesetSetDockVisible(false);

    blocks.clear();
    protocols.clear();
}

void DockItemDataManager::updateDockVisible()
{
    bool visible = blocks.count() > 0 || protocols.count() > 0;
    Q_EMIT requesetSetDockVisible(visible);
    qCInfo(logAppDock) << "dock entry visible:" << visible;
}

void DockItemDataManager::notify(const QString &title, const QString &msg)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QVariantList args;
    args << QString("dde-file-manager")
         << static_cast<uint>(0)
         << QString("drive-removable-dock")
         << title
         << msg
         << QStringList()
         << QVariantMap()
         << 3000;
    iface.asyncCallWithArgumentList("Notify", args);
}

DockItemData DockItemDataManager::buildBlockItem(const QVariantMap &data)
{
    QString displayName = device_utils::blockDeviceName(data);
    QString iconName = device_utils::blockDeviceIcon(data);
    QString id = data.value(GlobalServerDefines::DeviceProperty::kId).toString();
    QString backingID = data.value(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice).toString();
    if (backingID == "/")
        backingID = id;
    return {
        .id = id,
        .backingID = backingID,
        .displayName = displayName,
        .targetUrl = device_utils::blockDeviceTarget(data),
        .targetFileUrl = QUrl::fromLocalFile(data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString()),
        .iconName = iconName,
        .totalSize = data.value(GlobalServerDefines::DeviceProperty::kSizeTotal).toULongLong(),
        .usedSize = data.value(GlobalServerDefines::DeviceProperty::kSizeUsed).toULongLong(),
        .sortKey = QString("00%1_00%2").arg(iconName).arg(displayName),
        .isProtocolDevice = false
    };
}

DockItemData DockItemDataManager::buildProtocolItem(const QVariantMap &data)
{
    QString displayName = device_utils::protocolDeviceName(data);
    QString iconName = device_utils::protocolDeviceIcon(data);
    QString id = data.value(GlobalServerDefines::DeviceProperty::kId).toString();

    if (iconName == "phone" && (id.startsWith("gphoto") || id.startsWith("mtp")))
        iconName = "android-device";
    if (id.contains("Apple_Inc") || id.startsWith("afc"))
        iconName = "ios-device";

    return {
        .id = id,
        .backingID = id,
        .displayName = displayName,
        .targetUrl = device_utils::protocolDeviceTarget(data),
        .targetFileUrl = QUrl::fromLocalFile(data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString()),
        .iconName = iconName,
        .totalSize = data.value(GlobalServerDefines::DeviceProperty::kSizeTotal).toULongLong(),
        .usedSize = data.value(GlobalServerDefines::DeviceProperty::kSizeUsed).toULongLong(),
        .sortKey = QString("01%1_00%2").arg(iconName).arg(displayName),
        .isProtocolDevice = true
    };
}

void DockItemDataManager::initialize()
{
    qCInfo(logAppDock) << "start query block devices";
    auto reply = devMng->GetBlockDevicesIdList(GlobalServerDefines::DeviceQueryOption::kMounted);
    reply.waitForFinished();
    if (reply.isError()) {
        qCCritical(logAppDock) << "cannot obtain block devices from dbus!" << reply.error().message();
        return;
    }

    qCInfo(logAppDock) << "start query block info";
    QStringList blks = reply.value();
    for (const QString &blk : blks)
        onBlockMounted(blk);

    qCInfo(logAppDock) << "end query block devices";

    qCInfo(logAppDock) << "start query protocol devices";
    reply = devMng->GetProtocolDevicesIdList();
    reply.waitForFinished();
    if (reply.isError()) {
        qCCritical(logAppDock) << "cannot obtain protocol devices from dbus!" << reply.error().message();
        return;
    }
    qCInfo(logAppDock) << "start query protocol info";

    QStringList prots = reply.value();
    for (const QString &prot : prots)
        onProtocolMounted(prot);
    qCInfo(logAppDock) << "end query protocol devices";

    updateDockVisible();
}

void DockItemDataManager::ejectAll()
{
    devMng->DetachAllMountedDevices();
}

void DockItemDataManager::ejectDevice(const QString &id)
{
    if (id.startsWith("/org/freedesktop/")) {
        qCDebug(logAppDock) << "Ejecting block device:" << id;
        devMng->DetachBlockDevice(id);
    } else {
        qCDebug(logAppDock) << "Ejecting protocol device:" << id;
        devMng->DetachProtocolDevice(id);
    }
}

void DockItemDataManager::connectDeviceManger()
{
    connect(devMng.data(), &DeviceManager::BlockDriveAdded,
            this, [this] { this->playSoundOnDevPlugInOut(true); });
    connect(devMng.data(), &DeviceManager::BlockDriveRemoved,
            this, [this] { this->playSoundOnDevPlugInOut(false); });

    connect(devMng.data(), &DeviceManager::BlockDeviceMounted,
            this, &DockItemDataManager::onBlockMounted);
    connect(devMng.data(), &DeviceManager::BlockDeviceUnmounted,
            this, &DockItemDataManager::onBlockUnmounted);
    connect(devMng.data(), &DeviceManager::BlockDeviceRemoved,
            this, &DockItemDataManager::onBlockUnmounted);
    connect(devMng.data(), &DeviceManager::BlockDevicePropertyChanged,
            this, &DockItemDataManager::onBlockPropertyChanged);

    connect(devMng.data(), &DeviceManager::ProtocolDeviceMounted,
            this, &DockItemDataManager::onProtocolMounted);
    connect(devMng.data(), &DeviceManager::ProtocolDeviceUnmounted,
            this, &DockItemDataManager::onProtocolUnmounted);
    connect(devMng.data(), &DeviceManager::ProtocolDeviceRemoved,
            this, &DockItemDataManager::onProtocolUnmounted);

    connect(devMng.data(), &DeviceManager::SizeUsedChanged,
            this, [this](auto id, auto total, auto free) {
                Q_EMIT usageUpdated(id, total - free);
            });
    connect(devMng.data(), &DeviceManager::NotifyDeviceBusy,
            this, &DockItemDataManager::sendNotification);
}

void DockItemDataManager::watchService()
{
    auto watcher = new QDBusServiceWatcher(kDeviceDaemonName, QDBusConnection::sessionBus(),
                                           QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered,
            this, [this](auto serv) {
                qCInfo(logAppDock) << serv << "unregistered.";
                onServiceUnregistered();
            });
    connect(watcher, &QDBusServiceWatcher::serviceRegistered,
            this, [this](auto serv) {
                qCInfo(logAppDock) << serv << "registered.";
                onServiceRegistered();
            });
}

void DockItemDataManager::subscribeUsageMonitoring()
{
    QTimer::singleShot(0, this, [this]() {
        qCDebug(logAppDock) << "Dock plugin subscribing to device usage monitoring";
        devMng->StartMonitoringUsage();
    });
}

void DockItemDataManager::unsubscribeUsageMonitoring()
{
    QTimer::singleShot(0, this, [this]() {
        qCDebug(logAppDock) << "Dock plugin unsubscribing from device usage monitoring";
        devMng->StopMonitoringUsage();
    });
}

void DockItemDataManager::refreshUsage()
{
    QTimer::singleShot(0, this, [this]() {
        qInfo(logAppDock) << "Dock plugin requesting immediate device usage refresh";
        devMng->RefreshDeviceUsage();
    });
}
