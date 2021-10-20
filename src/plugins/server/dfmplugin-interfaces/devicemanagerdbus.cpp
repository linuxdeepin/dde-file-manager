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
    deviceServ->startConnect();
}

/*!
 * \brief pub signals with device monitor
 */
void DeviceManagerDBus::initConnection()
{
    connect(deviceServ, &DeviceService::blockDriveAdded, this, &DeviceManagerDBus::BlockDriveAdded);
    connect(deviceServ, &DeviceService::blockDriveRemoved, this, &DeviceManagerDBus::BlockDriveRemoved);
}

void DeviceManagerDBus::askStopAllDefenderScanning(int index, const QString &text)
{
    qInfo() << "ask stop " << index << text;
    if (index == 1) { // user clicked stop
        if (deviceServ->stopDefenderScanAllDrives()) {
            deviceServ->doUnMountAll();
        } else {
            dfmbase::UniversalUtils::notifyMessage(tr("The device was not safely removed"),
                                                   tr("Click \"Safely Remove\" and then disconnect it next time"));
        }
    }
}

void DeviceManagerDBus::UnmountAllDevices()
{
    if (deviceServ->isDefenderScanningDrive()) {
        // show query dialog
        auto &ctx = dpfInstance.serviceContext();
        dialogServ = ctx.service<DialogService>(DialogService::name());
        DDialog *d = dialogServ->showQueryScanningDialog(QObject::tr("Scanning the device, stop it?"));
        connect(d, &DDialog::buttonClicked, this, &DeviceManagerDBus::askStopAllDefenderScanning);
        return;
    }

    deviceServ->doUnMountAll();
}

QStringList DeviceManagerDBus::BlockDevicesIdList()
{
    // TODO(zhangs): wait dfm-mount impl
    return QStringList();
}

QString DeviceManagerDBus::QueryBlockDeviceInfo(QString id)
{
    // TODO(zhangs): build json
    return QString();
}

QStringList DeviceManagerDBus::ProtolcolDevicesIdList()
{
    // TODO(zhangs): wait dfm-mount impl
    return QStringList();
}

QString DeviceManagerDBus::QueryProtocolDeviceInfo(QString id)
{
    // TODO(zhangs): build json
    return QString();
}
