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

#include <QDBusInterface>
#include <QDebug>

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
    DevMngIns->detachBlockDev(id);
}

void DeviceManagerDBus::DetachProtocolDevice(QString id)
{
    DevMngIns->detachProtoDev(id);
}

void DeviceManagerDBus::initialize()
{
    DevMngIns->startMonitor();
    DevMngIns->startPollingDeviceUsage();
}

/*!
 * \brief pub signals with device monitor
 */
void DeviceManagerDBus::initConnection()
{
    connect(DevMngIns, &DeviceManager::blockDevUnmountAsyncFailed, this, [this](const QString &deviceId, DFMMOUNT::DeviceError err) {
        emit NotifyDeviceBusy(DeviceBusyAction::kUnmount);
    });
    connect(DevMngIns, &DeviceManager::blockDevEjectAsyncFailed, this, [this](const QString &deviceId, DFMMOUNT::DeviceError err) {
        emit NotifyDeviceBusy(DeviceBusyAction::kEject);
    });
    connect(DevMngIns, &DeviceManager::blockDevPoweroffAysncFailed, this, [this](const QString &deviceId, DFMMOUNT::DeviceError err) {
        emit NotifyDeviceBusy(DeviceBusyAction::kRemove);
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
        emit this->BlockDevicePropertyChanged(id, property, QDBusVariant(val));
    });

    connect(DevMngIns, &DeviceManager::protocolDevMounted, this, &DeviceManagerDBus::ProtocolDeviceMounted);
    connect(DevMngIns, &DeviceManager::protocolDevUnmounted, this, &DeviceManagerDBus::ProtocolDeviceUnmounted);
    connect(DevMngIns, &DeviceManager::protocolDevAdded, this, &DeviceManagerDBus::ProtocolDeviceAdded);
    connect(DevMngIns, &DeviceManager::protocolDevRemoved, this, &DeviceManagerDBus::ProtocolDeviceRemoved);

    auto refreshDesktop = [](const QString &msg) {
        qDebug() << "refresh desktop start..." << msg;
        QDBusInterface ifs("com.deepin.dde.desktop",
                           "/com/deepin/dde/desktop/canvas",
                           "com.deepin.dde.desktop.canvas");
        ifs.asyncCall("Refresh", false);
        qDebug() << "refresh desktop async finished..." << msg;
    };
    connect(DevMngIns, &DeviceManager::blockDevMounted, this, [this, refreshDesktop](const QString &id, const QString &mpt) {
        emit BlockDeviceMounted(id, mpt);
        refreshDesktop("onBlockDevMounted");
    });
    connect(DevMngIns, &DeviceManager::blockDevUnmounted, this, [this, refreshDesktop](const QString &id) {
        emit BlockDeviceUnmounted(id);
        refreshDesktop("onBlockDevUnmounted");
    });
    connect(DevMngIns, &DeviceManager::blockDevRemoved, this, [this, refreshDesktop](const QString &id) {
        emit BlockDeviceRemoved(id);
        refreshDesktop("onBlockDevRemoved");
    });
}

/*!
 * \brief this is convience interface for `disk-mount` plugin
 */
void DeviceManagerDBus::DetachAllMountedDevices()
{
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
