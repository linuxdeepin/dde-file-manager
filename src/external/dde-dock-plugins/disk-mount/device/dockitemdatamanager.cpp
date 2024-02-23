// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_server_defines.h"
#include "dockitemdatamanager.h"
#include "utils/dockutils.h"

#include <dtkgui_global.h>
#include <dtkwidget_global.h>
#include <DDesktopServices>

Q_DECLARE_LOGGING_CATEGORY(logAppDock)

DGUI_BEGIN_NAMESPACE
DGUI_END_NAMESPACE

static constexpr char kDeviceService[] { "org.deepin.filemanager.server" };
static constexpr char kDevMngPath[] { "/org/deepin/filemanager/server/DeviceManager" };
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
    devMng.reset(new DeviceManager(kDeviceService,
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
    if (data.value(GlobalServerDefines::DeviceProperty::kOpticalDrive).toBool())
        return kDisplay;
    if (data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString().isEmpty())
        return kIgnore;

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
    // dlnfs mounts will be captured by gvfs, ignore them.
    if (device_utils::isDlnfsMount(data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString()))
        return kIgnore;
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
    if (!blocks.contains(id))
        return;
    QString devName = blocks.value(id).displayName;
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
    devMng.reset(new DeviceManager(kDeviceService,
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
         << QString("media-eject")
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
        .sortKey = QString("00%1_00%2").arg(iconName).arg(displayName)
    };
}

DockItemData DockItemDataManager::buildProtocolItem(const QVariantMap &data)
{
    QString displayName = device_utils::protocolDeviceName(data);
    QString iconName = device_utils::protocolDeviceIcon(data);
    QString id = data.value(GlobalServerDefines::DeviceProperty::kId).toString();

    return {
        .id = id,
        .backingID = id,
        .displayName = displayName,
        .targetUrl = device_utils::protocolDeviceTarget(data),
        .targetFileUrl = QUrl::fromLocalFile(data.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString()),
        .iconName = iconName,
        .totalSize = data.value(GlobalServerDefines::DeviceProperty::kSizeTotal).toULongLong(),
        .usedSize = data.value(GlobalServerDefines::DeviceProperty::kSizeUsed).toULongLong(),
        .sortKey = QString("01%1_00%2").arg(iconName).arg(displayName)
    };
}

void DockItemDataManager::initialize()
{
    qCInfo(logAppDock) << "start query block devices";
    auto reply = devMng->GetBlockDevicesIdList(GlobalServerDefines::DeviceQueryOption::kMounted);
    reply.waitForFinished();
    if (reply.isError()) {
        qCritical() << "cannot obtain block devices from dbus!" << reply.error().message();
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
        qCritical() << "cannot obtain block devices from dbus!" << reply.error().message();
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
    if (id.startsWith("/org/freedesktop/"))
        devMng->DetachBlockDevice(id);
    else
        devMng->DetachProtocolDevice(id);
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
    auto watcher = new QDBusServiceWatcher(kDeviceService, QDBusConnection::sessionBus(),
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
