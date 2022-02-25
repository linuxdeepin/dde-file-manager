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
#ifndef DEVICEMANAGERDBUS_H
#define DEVICEMANAGERDBUS_H

#include "dfm-base/base/device/devicecontroller.h"

#include <QDBusVariant>
#include <QVariantMap>
#include <QObject>

class DeviceManagerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.service.DeviceManager")

public:
    explicit DeviceManagerDBus(QObject *parent = nullptr);
    ~DeviceManagerDBus();

signals:
    void AskStopSacnningWhenUnmount(QString id);
    void AskStopScanningWhenDetach(QString id);
    void AskStopScanningWhenDetachAll();
    void SizeUsedChanged(QString id, qint64 total, qint64 free);
    void NotifyDeviceBusy(int action);   // see "dfm-base/dbusservice/global_server_defines.h"

    void BlockDriveAdded();
    void BlockDriveRemoved();
    void BlockDeviceAdded(QString id);
    void BlockDeviceRemoved(QString id);
    void BlockDeviceFilesystemAdded(QString id);
    void BlockDeviceFilesystemRemoved(QString id);
    void BlockDeviceMounted(QString id, QString mountPoint);
    void BlockDeviceUnmounted(QString id);
    void BlockDevicePropertyChanged(QString id, QString property, QDBusVariant value);
    void BlockDeviceUnlocked(QString id, QString clearDeviceId);
    void BlockDeviceLocked(QString id);

    void ProtocolDeviceAdded(QString id);
    void ProtocolDeviceRemoved(QString id);
    void ProtocolDeviceMounted(QString id, QString mountPoint);
    void ProtocolDeviceUnmounted(QString id);

public slots:
    bool IsMonotorWorking();
    void SafelyRemoveBlockDevice(QString id);
    bool DetachBlockDevice(QString id);
    bool DetachBlockDeviceForced(QString id);
    bool DetachProtocolDevice(QString id);
    bool DetachAllMountedDevices();
    bool DetachAllMountedDevicesForced();

    QString MountBlockDevice(QString id);
    void UnmountBlockDevice(QString id);
    void UnmountBlockDeviceForced(QString id);
    bool RenameBlockDevice(QString id, QString newName);
    void EjectBlockDevice(QString id);
    void PoweroffBlockDevice(QString id);
    QString MountProtocolDevice(QString id);
    void UnmountProtocolDevice(QString id);
    QString UnlockBlockDevice(QString id, QString passwd);
    void LockBlockDevice(QString id);

    QStringList GetBlockDevicesIdList(const QVariantMap &opts);
    QVariantMap QueryBlockDeviceInfo(QString id, bool detail);
    QStringList GetProtocolDevicesIdList();
    QVariantMap QueryProtocolDeviceInfo(QString id, bool detail);

    void GhostBlockDevMounted(const QString &deviceId, const QString &mountPoint);

private:
    void initialize();
    void initConnection();

private:
    DFMBASE_NAMESPACE::DeviceController *deviceServ;
};

#endif   // DEVICEMANAGERDBUS_H
