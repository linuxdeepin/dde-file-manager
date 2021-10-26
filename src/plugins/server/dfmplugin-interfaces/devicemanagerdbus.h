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
#ifndef DEVICEMANAGERDBUS_H
#define DEVICEMANAGERDBUS_H

#include <QObject>

namespace dfm_service_common {
class DeviceService;
class DialogService;
}

class DeviceManagerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.service.DeviceManager")

public:
    explicit DeviceManagerDBus(QObject *parent = nullptr);
    ~DeviceManagerDBus();

signals:
    void BlockDriveAdded();
    void BlockDriveRemoved();
    void BlockDeviceAdded(const QString &deviceId);
    void BlockDeviceRemoved(const QString &deviceId);
    void BlockDeviceFilesystemAdded(const QString &deviceId);
    void BlockDeviceFilesystemRemoved(const QString &deviceId);
    void BlockDeviceMounted(const QString &deviceId, const QString &mountPoint);
    void BlockDeviceUnmounted(const QString &deviceId);

    // TODO(zhangs): signals of protocol devices

public slots:
    bool IsMonotorWorking();
    void EjectAllDevices();
    void EjectDevice(QString id);
    QStringList BlockDevicesIdList();
    QString QueryBlockDeviceInfo(QString id);
    QStringList ProtolcolDevicesIdList();
    QString QueryProtocolDeviceInfo(QString id);

private:
    void initialize();
    void initConnection();
    Q_SLOT void askStopAllDefenderScanning(int index, const QString &text);

private:
    dfm_service_common::DeviceService *deviceServ {nullptr};
    dfm_service_common::DialogService *dialogServ {nullptr};
};


#endif // DEVICEMANAGERDBUS_H
