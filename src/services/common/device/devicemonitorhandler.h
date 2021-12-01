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
#ifndef DEVICEMONITORHANDLER_H
#define DEVICEMONITORHANDLER_H

#include "dfm_common_service_global.h"
#include "deviceservicehelper.h"

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QMutex>

DSC_BEGIN_NAMESPACE

class DeviceService;
class DeviceMonitorHandler : public QObject
{
    friend class DeviceService;
    Q_OBJECT

private:
    explicit DeviceMonitorHandler(DeviceService *serv);
    void startMaintaining();
    void startConnect();
    void stopConnect();

    void initBlockDevicesData();
    void initProtocolDevicesData();
    bool insertNewBlockDeviceData(const DeviceServiceHelper::BlockDevPtr &ptr);
    bool insertNewProtocolDeviceData(const DeviceServiceHelper::ProtocolDevPtr &ptr);
    void removeBlockDeviceData(const QString &deviceId);
    void removeProtocolDeviceData(const QString &deviceId);
    void updateDataWithOpticalInfo(BlockDeviceData *data, const QMap<DFMMOUNT::Property, QVariant> &changes);
    void updateDataWithMountedInfo(BlockDeviceData *data, const QMap<DFMMOUNT::Property, QVariant> &changes);
    void updateDataWithOtherInfo(BlockDeviceData *data, const QMap<DFMMOUNT::Property, QVariant> &changes);
    void handleBlockDevicesSizeUsedChanged();
    void handleProtolDevicesSizeUsedChanged();

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
    void onDeviceSizeUsedTimeout();

private:
    static constexpr int kSizeUpdateInterval = 10000;   // timer interval: 10s

    QTimer sizeUpdateTimer;
    QPointer<DeviceService> service;
    QMutex mutexForBlock;
    QMutex mutexForProtocol;
    QHash<QString, BlockDeviceData> allBlockDevData;
    QHash<QString, ProtocolDeviceData> allProtocolDevData;
};

DSC_END_NAMESPACE

#endif   // DEVICEMONITORHANDLER_H
