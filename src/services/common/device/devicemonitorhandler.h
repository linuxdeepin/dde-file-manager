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
#ifndef DEVICEMONITORHANDLER_H
#define DEVICEMONITORHANDLER_H

#include "dfm_common_service_global.h"
#include "deviceservicehelper.h"

#include <QObject>
#include <QPointer>

DSC_BEGIN_NAMESPACE

class DeviceService;
class DeviceMonitorHandler : public QObject
{
    friend class DeviceService;
    Q_OBJECT

private:
    explicit DeviceMonitorHandler(DeviceService *serv);
    void startConnect();
    void stopConnect();

private slots:
    void onBlockDriveAdded(const QString &drvObjPath);
    void onBlockDriveRemoved(const QString &drvObjPath);
    void onBlockDeviceAdded(const QString &deviceId);
    void onBlockDeviceRemoved(const QString &deviceId);
    void onFilesystemAdded(const QString &deviceId);
    void onFilesystemRemoved(const QString &deviceId);
    void onBlockDeviceMounted(const QString &deviceId, const QString &mountPoint);
    void onBlockDeviceUnmounted(const QString &deviceId);
    void onBlockDevicePropertyChanged(const QString &deviceId, const QMap<DFMMOUNT::Property, QVariant> &changes);

private:
    QPointer<DeviceService> service;
};

DSC_END_NAMESPACE

#endif // DEVICEMONITORHANDLER_H
