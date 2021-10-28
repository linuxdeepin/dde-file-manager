/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "deviceservice.h"
#include "dialogservice.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>
#include <QtConcurrent>

DSC_USE_NAMESPACE

DeviceManagerDBus::DeviceManagerDBus(QObject *parent) : QObject(parent)
{
    initialize();
    initConnection();
}

DeviceManagerDBus::~DeviceManagerDBus()
{

}

bool DeviceManagerDBus::IsMonotorWorking()
{
    return deviceServ->isBlockDeviceMonitorWorking() && deviceServ->isProtolDeviceMonitorWorking();
}

void DeviceManagerDBus::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    deviceServ = ctx.service<DeviceService>(DeviceService::name());
    Q_ASSERT(deviceServ);
    deviceServ->startAutoMount();
    deviceServ->startMonitor();
}

/*!
 * \brief pub signals with device monitor
 */
void DeviceManagerDBus::initConnection()
{
    connect(deviceServ, &DeviceService::blockDriveAdded, this, &DeviceManagerDBus::BlockDriveAdded);
    connect(deviceServ, &DeviceService::blockDriveRemoved, this, &DeviceManagerDBus::BlockDriveRemoved);
    connect(deviceServ, &DeviceService::blockDeviceAdded, this, &DeviceManagerDBus::BlockDeviceAdded);
    connect(deviceServ, &DeviceService::blockDeviceRemoved, this, &DeviceManagerDBus::BlockDeviceRemoved);
    connect(deviceServ, &DeviceService::blockDeviceFilesystemAdded, this, &DeviceManagerDBus::BlockDeviceFilesystemAdded);
    connect(deviceServ, &DeviceService::blockDeviceFilesystemRemoved, this, &DeviceManagerDBus::BlockDeviceFilesystemRemoved);
    connect(deviceServ, &DeviceService::blockDeviceMounted, this, &DeviceManagerDBus::BlockDeviceMounted);
    connect(deviceServ, &DeviceService::blockDeviceUnmounted, this, &DeviceManagerDBus::BlockDeviceUnmounted);
}

void DeviceManagerDBus::askStopAllDefenderScanning(int index, const QString &text)
{
    qInfo() << "ask stop " << index << text;
    if (index == 1) { // user clicked stop
        if (deviceServ->stopDefenderScanAllDrives()) {
            deviceServ->ejectAllMountedDevices();
        } else {
            dfmbase::UniversalUtils::notifyMessage(tr("The device was not safely removed"),
                                                   tr("Click \"Safely Remove\" and then disconnect it next time"));
        }
    }
}

void DeviceManagerDBus::EjectAllMountedDevices()
{
    if (deviceServ->isDefenderScanningDrive()) {
        // show query dialog
        auto &ctx = dpfInstance.serviceContext();
        dialogServ = ctx.service<DialogService>(DialogService::name());
        DDialog *d = dialogServ->showQueryScanningDialog(QObject::tr("Scanning the device, stop it?"));
        connect(d, &DDialog::buttonClicked, this, &DeviceManagerDBus::askStopAllDefenderScanning);
        return;
    }

    deviceServ->ejectAllMountedDevices();
}

void DeviceManagerDBus::EjectDevice(QString id)
{
    deviceServ->eject(id);
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

QVariantMap DeviceManagerDBus::QueryBlockDeviceInfo(QString id)
{
    return deviceServ->blockDeviceInfo(id);
}

QStringList DeviceManagerDBus::GetProtolcolDevicesIdList()
{
    return deviceServ->protocolDevicesIdList();
}

QVariantMap DeviceManagerDBus::QueryProtocolDeviceInfo(QString id)
{
    // TODO(zhangs): build data
    return QVariantMap();
}
