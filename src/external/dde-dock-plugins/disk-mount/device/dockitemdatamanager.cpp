// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_server_defines.h"
#include "dockitemdatamanager.h"
#include "usbrepairproxy.h"
#include "utils/dockutils.h"
#include "widgets/repairdialog.h"

#include <dtkgui_global.h>
#include <dtkwidget_global.h>
#include <DDesktopServices>
#include <QTimer>
#include <QDBusPendingCallWatcher>
#include <QProcess>
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
    connectRepairService();
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
        notify("", tr("The device has been safely removed"));
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

void DockItemDataManager::notify(const QString &title, const QString &msg, int timeout)
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
         << timeout;
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

void DockItemDataManager::connectRepairService()
{
    m_repairProxy = new UsbRepairProxy(this);

    connect(m_repairProxy, &UsbRepairProxy::fsErrorDetected,
            this, &DockItemDataManager::onFsErrorDetected);
    connect(m_repairProxy, &UsbRepairProxy::fsErrorCleared,
            this, &DockItemDataManager::onFsErrorCleared);
    connect(m_repairProxy, &UsbRepairProxy::repairProgress,
            this, &DockItemDataManager::onRepairProgress);
    connect(m_repairProxy, &UsbRepairProxy::repairFinished,
            this, &DockItemDataManager::onRepairFinished);

    // Monitor notification actions (on session bus)
    QDBusConnection::sessionBus().connect(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "ActionInvoked",
        this,
        SLOT(onNotifyActionInvoked(uint, QString)));

    QDBusConnection::sessionBus().connect(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "NotificationClosed",
        this,
        SLOT(onNotifyClosed(uint, uint)));
}

void DockItemDataManager::onFsErrorDetected(
    const QString &devicePath,
    const QString &deviceName,
    const QString &fsType,
    const QString &errorType,
    bool canRepair,
    const QString &message)
{
    qCInfo(logAppDock) << "USB filesystem error detected:" << devicePath
                       << "fs:" << fsType << "type:" << errorType
                       << "canRepair:" << canRepair;

    // Store error info for later use (dialog needs it)
    m_pendingErrors[devicePath] = { deviceName, fsType, errorType, message };

    if (canRepair) {
        QString title = tr("Detect Device Abnormality, Repair Recommended");
        QString msg = tr("Your %1 has data errors, possibly caused by unsafe removal!")
                          .arg(deviceName);
        QStringList actions;
        actions << "repair" << tr("Repair Immediately")
                << "ignore" << tr("Ignore for Now");
        notifyWithActions(title, msg, actions, devicePath);
    } else if (errorType == "read_only") {
        // Hardware read-only: info-only notification, no repair button
        QString title = tr("Hardware Failure Warning");
        QString msg = tr("%1 is in hardware write-protected mode.\n"
                         "Flash memory may be failing. Please back up data immediately.")
                          .arg(deviceName);
        notify(title, msg, 15000);  // 15 seconds
    } else {
        // Other non-repairable errors (e.g., unknown filesystem)
        QString title = tr("Device Damaged, Cannot Auto Repair");
        QString msg = tr("The data structure of %1 is severely damaged and cannot be recognized or repaired by the system. If there are important files inside, please stop using it. It is recommended to use professional data recovery software or seek professional assistance.")
                          .arg(deviceName);
        notify(title, msg, 15000);  // 15 seconds
    }
}

void DockItemDataManager::onFsErrorCleared(const QString &devicePath)
{
    qCInfo(logAppDock) << "USB filesystem error cleared:" << devicePath;
    m_pendingErrors.remove(devicePath);

    // Find and close the associated notification
    uint nid = 0;
    for (auto it = m_notificationToDevice.begin(); it != m_notificationToDevice.end(); ++it) {
        if (it.value() == devicePath) {
            nid = it.key();
            break;
        }
    }
    if (nid > 0) {
        QDBusInterface iface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications",
                             QDBusConnection::sessionBus());
        iface.asyncCall("CloseNotification", nid);
        m_notificationToDevice.remove(nid);
    }
}

void DockItemDataManager::onNotifyActionInvoked(uint notificationId, const QString &action)
{
    qCInfo(logAppDock) << "Notification action invoked:" << notificationId << action;

    if (!m_notificationToDevice.contains(notificationId))
        return;

    QString devicePath = m_notificationToDevice.value(notificationId);

    if (action == "repair") {
        closeNotification(notificationId);

        auto it = m_pendingErrors.find(devicePath);
        if (it != m_pendingErrors.end()) {
            const PendingErrorInfo &info = it.value();
            QString deviceSize = blocks.contains(devicePath) ?
                size_format::formatDiskSize(blocks.value(devicePath).totalSize) : "";

            // Launch standalone repair dialog process. The tray-loader process
            // cannot create top-level QWidget windows under dde-shell's
            // custom Wayland protocol, so the dialog must run in its own
            // process as a regular Wayland client.
            QProcess::startDetached(QStringLiteral("dde-usb-repair-dialog"), {
                QStringLiteral("-d"), devicePath,
                QStringLiteral("-n"), info.deviceName,
                QStringLiteral("-f"), info.fsType,
                QStringLiteral("-s"), deviceSize
            });
        }
    } else if (action == "ignore") {
        // User chose to ignore - just clean up
        closeNotification(notificationId);
        m_pendingErrors.remove(devicePath);
    }

    m_notificationToDevice.remove(notificationId);
}

void DockItemDataManager::onNotifyClosed(uint notificationId, uint reason)
{
    Q_UNUSED(reason)
    // Do NOT remove from m_notificationToDevice here.
    // The notification daemon may emit NotificationClosed before ActionInvoked
    // (or in response to the action click), which would clear the map entry
    // before onNotifyActionInvoked can look it up. Let onNotifyActionInvoked
    // handle the removal instead.
}

void DockItemDataManager::notifyWithActions(const QString &title, const QString &msg,
                                              const QStringList &actions, const QString &devicePath, int timeout)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QVariantList args;
    args << QString("dde-file-manager")
         << static_cast<uint>(0)
         << QString("dde-file-manager")
         << title
         << msg
         << actions
         << QVariantMap()
         << timeout;
    QDBusPendingCall async = iface.asyncCallWithArgumentList("Notify", args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, devicePath](QDBusPendingCallWatcher *w) {
                QDBusPendingReply<uint> reply = *w;
                if (!reply.isError()) {
                    uint notificationId = reply.value();
                    m_notificationToDevice[notificationId] = devicePath;
                    qCDebug(logAppDock) << "USB repair notification sent, id:"
                                        << notificationId << "device:" << devicePath;
                } else {
                    qCWarning(logAppDock) << "Failed to send notification:"
                                          << reply.error().message();
                }
                w->deleteLater();
            });
}

void DockItemDataManager::closeNotification(uint notificationId)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    iface.asyncCall("CloseNotification", notificationId);
}

void DockItemDataManager::onRepairProgress(const QString &devicePath, int percent, const QString &logLine)
{
    qCInfo(logAppDock) << "Repair progress:" << devicePath << percent << "%" << logLine;

    // The dialog was already created and is running exec() in its kRepairing state.
    // Just update the progress.
    RepairDialog *dialog = m_repairDialogs.value(devicePath);
    if (dialog) {
        dialog->setProgress(percent);
    }
}

void DockItemDataManager::onRepairFinished(const QString &devicePath, bool success, const QString &summary)
{
    qCInfo(logAppDock) << "Repair finished:" << devicePath << "success:" << success << "summary:" << summary;

    // Get device info from pending errors
    auto it = m_pendingErrors.find(devicePath);
    QString deviceName = it != m_pendingErrors.end() ? it.value().deviceName : devicePath;
    QString fsType = it != m_pendingErrors.end() ? it.value().fsType : "";

    // When using the standalone process (dde-usb-repair-dialog), it handles
    // mounting and dialog UI itself. We only need to clean up pending errors.
    RepairDialog *dialog = m_repairDialogs.value(devicePath);
    if (!dialog) {
        m_pendingErrors.remove(devicePath);
        return;
    }

    // In-process dialog path: mount and update dialog state.
    QString mountPoint;
    if (success) {
        qCInfo(logAppDock) << "Attempting to mount device after repair:" << devicePath;

        // Use udisksctl to mount the device
        QProcess mountProc;
        mountProc.start("udisksctl", { "mount", "-b", devicePath });
        mountProc.waitForFinished(10000);

        if (mountProc.exitCode() != 0) {
            QString err = QString::fromUtf8(mountProc.readAllStandardError());
            qCWarning(logAppDock) << "Mount failed:" << err;
            success = false;  // Mount failed, treat as overall failure
        }

        // Read /proc/mounts to get accurate mount point
        QFile mounts("/proc/mounts");
        if (mounts.open(QIODevice::ReadOnly)) {
            QByteArray data = mounts.readAll();
            mounts.close();
            for (const QByteArray &line : data.split('\n')) {
                QList<QByteArray> parts = line.split(' ');
                if (parts.size() >= 2 && parts[0] == devicePath.toUtf8()) {
                    mountPoint = QString::fromUtf8(parts[1]);
                    qCInfo(logAppDock) << "Mount point from /proc/mounts:" << mountPoint;
                    break;
                }
            }
        }

        // If mount point is empty, mount failed
        if (mountPoint.isEmpty()) {
            qCWarning(logAppDock) << "Mount point is empty after mount attempt";
            success = false;
        }
    }

    if (dialog) {
        QString deviceSize = blocks.contains(devicePath) ?
            size_format::formatDiskSize(blocks.value(devicePath).totalSize) : "";
        dialog->setDeviceInfo(deviceName, deviceSize, fsType.toUpper());
        dialog->setDevicePath(devicePath, mountPoint);

        if (success) {
            dialog->setState(RepairDialog::kSuccess);

            // Handle "Open Device" button
            connect(dialog, &RepairDialog::buttonClicked, this, [dialog](int index, const QString &) {
                if (index == 1) {  // "Open Device" button (second button)
                    QString mountPoint = dialog->mountPoint();
                    if (!mountPoint.isEmpty()) {
                        qCInfo(logAppDock) << "Opening device:" << mountPoint;
                        QProcess::startDetached(QStringLiteral("dde-file-manager"), { mountPoint });
                    }
                }
            });
        } else {
            // 设置错误码（包含修复失败的原因或挂载失败的错误）
            QString errorCode = summary;
            if (errorCode.isEmpty()) {
                errorCode = tr("Repair operation failed");
            }
            dialog->setErrorCode(errorCode);
            dialog->setState(RepairDialog::kFailed);
        }
    }

    // Clean up pending errors; dialog will be deleted after exec() returns
    m_pendingErrors.remove(devicePath);
}
