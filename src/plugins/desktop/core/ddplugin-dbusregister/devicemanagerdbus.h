// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEMANAGERDBUS_H
#define DEVICEMANAGERDBUS_H

#include <dfm-base/base/device/devicemanager.h>

#include <QDBusVariant>
#include <QVariantMap>
#include <QObject>

class DeviceManagerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.filemanager.server.DeviceManager")

public:
    explicit DeviceManagerDBus(QObject *parent = nullptr);

signals:
    void SizeUsedChanged(QString id, qint64 total, qint64 free);
    void NotifyDeviceBusy(int action);   // see "dfm-base/dbusservice/global_server_defines.h"

    void BlockDriveAdded();
    void BlockDriveRemoved();
    void BlockDeviceAdded(QString id);
    void BlockDeviceRemoved(QString id, const QString &oldMpt);
    void BlockDeviceFilesystemAdded(QString id);
    void BlockDeviceFilesystemRemoved(QString id);
    void BlockDeviceMounted(QString id, QString mountPoint);
    void BlockDeviceUnmounted(QString id, const QString &oldMpt);
    void BlockDevicePropertyChanged(QString id, QString property, QDBusVariant value);
    void BlockDeviceUnlocked(QString id, QString clearDeviceId);
    void BlockDeviceLocked(QString id);

    void ProtocolDeviceAdded(QString id);
    void ProtocolDeviceRemoved(QString id, const QString &oldMpt);
    void ProtocolDeviceMounted(QString id, QString mountPoint);
    void ProtocolDeviceUnmounted(QString id, const QString &oldMpt);

public slots:
    bool IsMonotorWorking();
    void DetachBlockDevice(QString id);
    void DetachProtocolDevice(QString id);
    void DetachAllMountedDevices();

    QStringList GetBlockDevicesIdList(int opts);
    QVariantMap QueryBlockDeviceInfo(QString id, bool reload);
    QStringList GetProtocolDevicesIdList();
    QVariantMap QueryProtocolDeviceInfo(QString id, bool reload);

private:
    void initialize();
    void initConnection();
};

#endif   // DEVICEMANAGERDBUS_H
