// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicemanagerdbus.h"
#include "daemonplugin_core_global.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-io/denumerator.h>
#include <dfm-io/dfileinfo.h>

#include <QDBusInterface>
#include <QDebug>

DAEMONPCORE_USE_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(DAEMONPCORE_NAMESPACE)
DFMBASE_USE_NAMESPACE

using namespace GlobalServerDefines;

DeviceManagerDBus::DeviceManagerDBus(QObject *parent)
    : QObject(parent)
{
    initialize();
    initConnection();
    DevMngIns->doAutoMountAtStart();
}

bool DeviceManagerDBus::IsMonotorWorking()
{
    return DevMngIns->isMonitoring();
}

void DeviceManagerDBus::DetachBlockDevice(QString id)
{
    fmInfo() << "[DeviceManagerDBus] Detaching block device:" << id;
    DevMngIns->detachBlockDev(id);
}

void DeviceManagerDBus::DetachProtocolDevice(QString id)
{
    fmInfo() << "[DeviceManagerDBus] Detaching protocol device:" << id;
    DevMngIns->detachProtoDev(id);
}

void DeviceManagerDBus::initialize()
{
    fmInfo() << "[DeviceManagerDBus] Initializing device manager";
    DevMngIns->startMonitor();

    // 改为仅初始化缓存，不启动轮询
    fmInfo() << "[DeviceManagerDBus] Initializing usage cache (polling will start on-demand)";
    DevMngIns->initUsageCache();

    DevMngIns->enableBlockAutoMount();

    // 新增：监听客户端断开连接
    QDBusConnection::sessionBus().connect(
            QString(),
            QString(),
            "org.freedesktop.DBus",
            "NameOwnerChanged",
            this,
            SLOT(onNameOwnerChanged(QString, QString, QString)));
}

/*!
 * \brief pub signals with device monitor
 */
void DeviceManagerDBus::initConnection()
{
    connect(DevMngIns, &DeviceManager::blockDevUnmountAsyncFailed, this, [this](auto deviceId) {
        fmWarning() << "[DeviceManagerDBus] Block device unmount failed:" << deviceId;
        emit NotifyDeviceBusy(deviceId, DeviceBusyAction::kUnmount);
    });
    connect(DevMngIns, &DeviceManager::blockDevEjectAsyncFailed, this, [this](auto deviceId) {
        fmWarning() << "[DeviceManagerDBus] Block device eject failed:" << deviceId;
        emit NotifyDeviceBusy(deviceId, DeviceBusyAction::kEject);
    });
    connect(DevMngIns, &DeviceManager::blockDevPoweroffAysncFailed, this, [this](auto deviceId) {
        fmWarning() << "[DeviceManagerDBus] Block device power off failed:" << deviceId;
        emit NotifyDeviceBusy(deviceId, DeviceBusyAction::kPowerOff);
    });
    connect(DevMngIns, &DeviceManager::protocolDevUnmountAsyncFailed, this, [this](auto deviceId) {
        fmWarning() << "[DeviceManagerDBus] Protocol device unmount failed:" << deviceId;
        emit NotifyDeviceBusy(deviceId, DeviceBusyAction::kUnmount);
    });

    connect(DevMngIns, &DeviceManager::devSizeChanged, this, &DeviceManagerDBus::SizeUsedChanged);
    connect(DevMngIns, &DeviceManager::blockDriveAdded, this, &DeviceManagerDBus::BlockDriveAdded);
    connect(DevMngIns, &DeviceManager::blockDriveRemoved, this, &DeviceManagerDBus::BlockDriveRemoved);
    connect(DevMngIns, &DeviceManager::blockDevAdded, this, &DeviceManagerDBus::BlockDeviceAdded);
    connect(DevMngIns, &DeviceManager::blockDevFsAdded, this, &DeviceManagerDBus::BlockDeviceFilesystemAdded);
    connect(DevMngIns, &DeviceManager::blockDevFsRemoved, this, &DeviceManagerDBus::BlockDeviceFilesystemRemoved);
    connect(DevMngIns, &DeviceManager::blockDevUnlocked, this, &DeviceManagerDBus::BlockDeviceUnlocked);
    connect(DevMngIns, &DeviceManager::blockDevLocked, this, &DeviceManagerDBus::BlockDeviceLocked);
    connect(DevMngIns, &DeviceManager::blockDevPropertyChanged, this, [this](const QString &id, const QString &property, const QVariant &val) {
        if (!val.isNull() && val.isValid()) {
            fmDebug() << "[DeviceManagerDBus] Block device property changed - ID:" << id << "property:" << property;
            emit this->BlockDevicePropertyChanged(id, property, QDBusVariant(val));
        }
    });

    connect(DevMngIns, &DeviceManager::protocolDevAdded, this, &DeviceManagerDBus::ProtocolDeviceAdded);
    connect(DevMngIns, &DeviceManager::protocolDevMounted, this, [this](const QString &id, const QString &mpt) {
        fmInfo() << "[DeviceManagerDBus] Protocol device mounted - ID:" << id << "mount point:" << mpt;
        emit ProtocolDeviceMounted(id, mpt);
        requestRefreshDesktopAsNeeded(mpt, "onMount");
    });
    connect(DevMngIns, &DeviceManager::protocolDevUnmounted, this, [this](const QString &id, const QString &oldMpt) {
        fmInfo() << "[DeviceManagerDBus] Protocol device unmounted - ID:" << id << "previous mount point:" << oldMpt;
        emit ProtocolDeviceUnmounted(id, oldMpt);
        requestRefreshDesktopAsNeeded(oldMpt, "onUnmount");
    });
    connect(DevMngIns, &DeviceManager::protocolDevRemoved, this, [this](const QString &id, const QString &oldMpt) {
        fmInfo() << "[DeviceManagerDBus] Protocol device removed - ID:" << id << "previous mount point:" << oldMpt;
        emit ProtocolDeviceRemoved(id, oldMpt);
        requestRefreshDesktopAsNeeded(oldMpt, "onRemove");
    });

    connect(DevMngIns, &DeviceManager::blockDevMounted, this, [this](const QString &id, const QString &mpt) {
        fmInfo() << "[DeviceManagerDBus] Block device mounted - ID:" << id << "mount point:" << mpt;
        emit BlockDeviceMounted(id, mpt);
        requestRefreshDesktopAsNeeded(mpt, "onMount");
    });
    connect(DevMngIns, &DeviceManager::blockDevUnmounted, this, [this](const QString &id, const QString &oldMpt) {
        fmInfo() << "[DeviceManagerDBus] Block device unmounted - ID:" << id << "previous mount point:" << oldMpt;
        emit BlockDeviceUnmounted(id, oldMpt);
        requestRefreshDesktopAsNeeded(oldMpt, "onUnmount");
    });
    connect(DevMngIns, &DeviceManager::blockDevRemoved, this, [this](const QString &id, const QString &oldMpt) {
        fmInfo() << "[DeviceManagerDBus] Block device removed - ID:" << id << "previous mount point:" << oldMpt;
        emit BlockDeviceRemoved(id, oldMpt);
        requestRefreshDesktopAsNeeded(oldMpt, "onRemove");
    });
}

void DeviceManagerDBus::requestRefreshDesktopAsNeeded(const QString &path, const QString &operation)
{
    QString desktopPath = StandardPaths::location(StandardPaths::kDesktopPath);
    if (desktopPath.isEmpty() || path.isEmpty()) {
        fmDebug() << "[DeviceManagerDBus] Skipping desktop refresh - empty desktop path or device path";
        return;
    }

    fmDebug() << "[DeviceManagerDBus] Checking for desktop links targeting path:" << path;
    dfmio::DEnumerator enu(QUrl::fromLocalFile(desktopPath));
    auto files = enu.fileInfoList();
    bool hasFileLinkToTarget = std::any_of(files.cbegin(), files.cend(), [path](QSharedPointer<dfmio::DFileInfo> file) {
        bool isSym = file->attribute(dfmio::DFileInfo::AttributeID::kStandardIsSymlink).toBool();
        if (!isSym) return false;
        auto target = file->attribute(dfmio::DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
        return target.startsWith(path);
    });

    if (hasFileLinkToTarget) {
        fmInfo() << "[DeviceManagerDBus] Found desktop links targeting device path, scheduling desktop refresh - operation:" << operation << "path:" << path;
        // send refresh request delay 3s which walkaround the device is moounting,such as ntfs.
        QTimer::singleShot(3 * 1000, []() {
            QDBusInterface ifs("com.deepin.dde.desktop",
                               "/com/deepin/dde/desktop",
                               "com.deepin.dde.desktop");
            ifs.asyncCall("Refresh");
        });
    } else {
        fmDebug() << "[DeviceManagerDBus] No desktop links found targeting device path, skipping refresh - path:" << path;
    }
}

/*!
 * \brief this is convience interface for `disk-mount` plugin
 */
void DeviceManagerDBus::DetachAllMountedDevices()
{
    fmInfo() << "[DeviceManagerDBus] Detaching all mounted devices";
    DevMngIns->detachAllRemovableBlockDevs();
    DevMngIns->detachAllProtoDevs();
}

/*!
 * \brief user input a opts, then return devices list
 * \param opts: refrecne to DeviceService::blockDevicesIdList
 * \return devices id list
 */
QStringList DeviceManagerDBus::GetBlockDevicesIdList(int opts)
{
    return DevMngIns->getAllBlockDevID(static_cast<DeviceQueryOptions>(opts));
}

QVariantMap DeviceManagerDBus::QueryBlockDeviceInfo(QString id, bool reload)
{
    return DevMngIns->getBlockDevInfo(id, reload);
}

QStringList DeviceManagerDBus::GetProtocolDevicesIdList()
{
    return DevMngIns->getAllProtocolDevID();
}

QVariantMap DeviceManagerDBus::QueryProtocolDeviceInfo(QString id, bool reload)
{
    return DevMngIns->getProtocolDevInfo(id, reload);
}

void DeviceManagerDBus::StartMonitoringUsage()
{
    // 获取调用者的 DBus 唯一名称
    QString clientName = message().service();

    if (clientName.isEmpty()) {
        fmWarning() << "[DeviceManagerDBus] StartMonitoringUsage called with empty client name, ignored";
        return;
    }

    // 检查是否已订阅（防止重复）
    if (m_monitoringClients.contains(clientName)) {
        fmDebug() << "[DeviceManagerDBus] Client already monitoring, ignored:" << clientName;
        return;
    }

    // 记录订阅前状态
    bool wasEmpty = m_monitoringClients.isEmpty();

    // 添加到订阅集合（引用计数 +1）
    m_monitoringClients.insert(clientName);

    fmInfo() << "[DeviceManagerDBus] Client started monitoring usage:" << clientName
             << "| Total clients:" << m_monitoringClients.size();

    // 引用计数从 0 变为 1：启动轮询
    if (wasEmpty) {
        fmInfo() << "[DeviceManagerDBus] First client subscribed, starting device usage polling";
        DevMngIns->startPollingDeviceUsage();
        // 注意：startPollingDeviceUsage() 内部会立即调用一次 queryUsageAsync()
    } else {
        fmDebug() << "[DeviceManagerDBus] Polling already active, no action needed";
    }
}

void DeviceManagerDBus::StopMonitoringUsage()
{
    // 获取调用者的 DBus 唯一名称
    QString clientName = message().service();

    if (clientName.isEmpty()) {
        fmWarning() << "[DeviceManagerDBus] StopMonitoringUsage called with empty client name, ignored";
        return;
    }

    // 尝试移除订阅
    if (!m_monitoringClients.remove(clientName)) {
        fmWarning() << "[DeviceManagerDBus] Client was not monitoring, ignored:" << clientName;
        return;
    }

    fmInfo() << "[DeviceManagerDBus] Client stopped monitoring usage:" << clientName
             << "| Remaining clients:" << m_monitoringClients.size();

    // 引用计数归零：停止轮询
    if (m_monitoringClients.isEmpty()) {
        fmInfo() << "[DeviceManagerDBus] All clients unsubscribed, stopping device usage polling";
        DevMngIns->stopPollingDeviceUsage();
    } else {
        fmDebug() << "[DeviceManagerDBus] Other clients still monitoring, keeping polling active";
    }
}

void DeviceManagerDBus::RefreshDeviceUsage()
{
    // 防抖检查：500ms 内的重复调用忽略
    if (m_lastRefreshTimer.isValid() && m_lastRefreshTimer.elapsed() < kRefreshDebounceMs) {
        fmDebug() << "[DeviceManagerDBus] RefreshDeviceUsage called within debounce interval, ignored";
        return;
    }

    fmInfo() << "[DeviceManagerDBus] Refreshing device usage on client request";

    // 先执行刷新
    DevMngIns->refreshUsage();

    // 刷新成功后才重置计时器
    m_lastRefreshTimer.start();
}

void DeviceManagerDBus::onNameOwnerChanged(const QString &name,
                                           const QString &oldOwner,
                                           const QString &newOwner)
{
    // 仅处理客户端断开连接的情况（newOwner 为空）
    if (!newOwner.isEmpty())
        return;

    // 检查是否是订阅客户端
    if (!m_monitoringClients.contains(name))
        return;

    // 客户端异常断开，自动清理
    fmWarning() << "[DeviceManagerDBus] Client disconnected unexpectedly, auto cleanup:" << name
                << "| Old owner:" << oldOwner;

    m_monitoringClients.remove(name);

    fmInfo() << "[DeviceManagerDBus] Remaining monitoring clients:" << m_monitoringClients.size();

    // 如果所有客户端都断开，停止轮询
    if (m_monitoringClients.isEmpty()) {
        fmInfo() << "[DeviceManagerDBus] All clients disconnected, stopping device usage polling";
        DevMngIns->stopPollingDeviceUsage();
    }
}
