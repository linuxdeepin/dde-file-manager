/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "devicemanagerdbus.h"

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"

DFMBASE_USE_NAMESPACE

using namespace GlobalServerDefines;

DeviceManagerDBus::DeviceManagerDBus(QObject *parent)
    : QObject(parent),
      deviceServ(DeviceController::instance())
{
    initialize();
    initConnection();
}

bool DeviceManagerDBus::IsMonotorWorking()
{
    return deviceServ->isBlockDeviceMonitorWorking() && deviceServ->isProtolDeviceMonitorWorking();
}

void DeviceManagerDBus::SafelyRemoveBlockDevice(QString id)
{
    DetachBlockDevice(id);
    // only optical eject
    connect(deviceServ, &DeviceController::blockDevAsyncEjected, this, [this, id](const QString &deviceId, bool success) {
        if (deviceId == id && success)
            deviceServ->poweroffBlockDeviceAsync(id);
    });
}

bool DeviceManagerDBus::DetachBlockDevice(QString id)
{
    bool ret = deviceServ->detachBlockDevice(id);
    if (!ret)
        emit NotifyDeviceBusy(DeviceBusyAction::kUnmount);
    return ret;
}

bool DeviceManagerDBus::DetachBlockDeviceForced(QString id)
{
    if (deviceServ->stopDefenderScanDrive(id)) {
        return deviceServ->detachBlockDevice(id);
    } else {
        emit NotifyDeviceBusy(DeviceBusyAction::kSafelyRemove);
        return false;
    }
}

bool DeviceManagerDBus::DetachProtocolDevice(QString id)
{
    return deviceServ->detachProtocolDevice(id);
}

void DeviceManagerDBus::initialize()
{
    deviceServ->startMonitor();
    deviceServ->startAutoMount();
}

/*!
 * \brief pub signals with device monitor
 */
void DeviceManagerDBus::initConnection()
{
    connect(deviceServ, &DeviceController::blockDevicePropertyChanged, this, [this](const QString &deviceId, const QString &property, const QVariant &val) {
        emit BlockDevicePropertyChanged(deviceId, property, QDBusVariant(val));
    });
    connect(deviceServ, &DeviceController::blockDevAsyncUnmounted, this, [this](const QString &deviceId, bool success) {
        if (!deviceId.isEmpty() && !success)
            emit NotifyDeviceBusy(DeviceBusyAction::kUnmount);
    });
    connect(deviceServ, &DeviceController::blockDevAsyncEjected, this, [this](const QString &deviceId, bool success) {
        if (!deviceId.isEmpty() && !success)
            emit NotifyDeviceBusy(DeviceBusyAction::kEject);
    });
    connect(deviceServ, &DeviceController::blockDevAsyncPoweroffed, this, [this](const QString &deviceId, bool success) {
        if (!deviceId.isEmpty() && !success)
            emit NotifyDeviceBusy(DeviceBusyAction::kRemove);
    });

    connect(deviceServ, &DeviceController::deviceSizeUsedChanged, this, &DeviceManagerDBus::SizeUsedChanged);
    connect(deviceServ, &DeviceController::blockDriveAdded, this, &DeviceManagerDBus::BlockDriveAdded);
    connect(deviceServ, &DeviceController::blockDriveRemoved, this, &DeviceManagerDBus::BlockDriveRemoved);
    connect(deviceServ, &DeviceController::blockDevAdded, this, &DeviceManagerDBus::BlockDeviceAdded);
    connect(deviceServ, &DeviceController::blockDevRemoved, this, &DeviceManagerDBus::BlockDeviceRemoved);
    connect(deviceServ, &DeviceController::blockDevFilesystemAdded, this, &DeviceManagerDBus::BlockDeviceFilesystemAdded);
    connect(deviceServ, &DeviceController::blockDevFilesystemRemoved, this, &DeviceManagerDBus::BlockDeviceFilesystemRemoved);
    connect(deviceServ, &DeviceController::blockDevMounted, this, &DeviceManagerDBus::BlockDeviceMounted);
    connect(deviceServ, &DeviceController::blockDevUnmounted, this, &DeviceManagerDBus::BlockDeviceUnmounted);
    connect(deviceServ, &DeviceController::blockDevUnlocked, this, &DeviceManagerDBus::BlockDeviceUnlocked);
    connect(deviceServ, &DeviceController::blockDevLocked, this, &DeviceManagerDBus::BlockDeviceLocked);

    connect(deviceServ, &DeviceController::protocolDevMounted, this, &DeviceManagerDBus::ProtocolDeviceMounted);
    connect(deviceServ, &DeviceController::protocolDevUnmounted, this, &DeviceManagerDBus::ProtocolDeviceUnmounted);
    connect(deviceServ, &DeviceController::protocolDevAdded, this, &DeviceManagerDBus::ProtocolDeviceAdded);
    connect(deviceServ, &DeviceController::protocolDevRemoved, this, &DeviceManagerDBus::ProtocolDeviceRemoved);
}

/*!
 * \brief this is convience interface for `disk-mount` plugin
 */
bool DeviceManagerDBus::DetachAllMountedDevices()
{
    if (deviceServ->isDefenderScanningDrive()) {
        // show query dialog
        emit AskStopScanningWhenDetachAll();
        return false;
    }

    bool ret = deviceServ->detachAllMountedBlockDevices()
            && deviceServ->detachAllMountedProtocolDevices();
    if (!ret)
        emit NotifyDeviceBusy(DeviceBusyAction::kUnmount);
    return ret;
}

bool DeviceManagerDBus::DetachAllMountedDevicesForced()
{
    if (deviceServ->stopDefenderScanAllDrives()) {
        return deviceServ->detachAllMountedBlockDevices()
                && deviceServ->detachAllMountedProtocolDevices();
    } else {
        emit NotifyDeviceBusy(DeviceBusyAction::kSafelyRemove);
        return false;
    }
}

QString DeviceManagerDBus::MountBlockDevice(QString id)
{
    return deviceServ->mountBlockDevice(id);
}

void DeviceManagerDBus::UnmountBlockDevice(QString id)
{
    if (deviceServ->isDefenderScanningDrive(id)) {
        // show query dialog
        emit AskStopSacnningWhenUnmount(id);
        return;
    }

    deviceServ->unmountBlockDeviceAsync(id);
}

void DeviceManagerDBus::UnmountBlockDeviceForced(QString id)
{
    if (deviceServ->stopDefenderScanDrive(id)) {
        deviceServ->unmountBlockDeviceAsync(id);
    } else {
        emit NotifyDeviceBusy(DeviceBusyAction::kSafelyRemove);
    }
}

bool DeviceManagerDBus::RenameBlockDevice(QString id, QString newName)
{
    // TODO(xust) this may take a long time while renaming ext* series filesystem, consider using async version instead.
    return deviceServ->renameBlockDevice(id, newName);
}

void DeviceManagerDBus::EjectBlockDevice(QString id)
{
    deviceServ->ejectBlockDeviceAsync(id);
}

void DeviceManagerDBus::PoweroffBlockDevice(QString id)
{
    deviceServ->poweroffBlockDeviceAsync(id);
}

QString DeviceManagerDBus::MountProtocolDevice(QString id)
{
    return deviceServ->mountProtocolDevice(id);
}

void DeviceManagerDBus::UnmountProtocolDevice(QString id)
{
    deviceServ->unmountProtocolDeviceAsync(id);
}

QString DeviceManagerDBus::UnlockBlockDevice(QString id, QString passwd)
{
    return deviceServ->unlockBlockDevice(passwd, id);
}

void DeviceManagerDBus::LockBlockDevice(QString id)
{
    deviceServ->lockBlockDevice(id);
}

/*!
 * \brief user input a opts, then return devices list
 * \param opts: refrecne to DeviceService::blockDevicesIdList
 * \return devices id list
 */
QStringList DeviceManagerDBus::GetBlockDevicesIdList(const QVariantMap &opts)
{
    return deviceServ->blockDevicesIdList(opts);
}

QVariantMap DeviceManagerDBus::QueryBlockDeviceInfo(QString id, bool detail)
{
    return deviceServ->blockDeviceInfo(id, detail);
}

QStringList DeviceManagerDBus::GetProtocolDevicesIdList()
{
    return deviceServ->protocolDevicesIdList();
}

QVariantMap DeviceManagerDBus::QueryProtocolDeviceInfo(QString id, bool detail)
{
    return deviceServ->protocolDeviceInfo(id, detail);
}

void DeviceManagerDBus::GhostBlockDevMounted(const QString &deviceId, const QString &mountPoint)
{
    return deviceServ->ghostBlockDevMounted(deviceId, mountPoint);
}
