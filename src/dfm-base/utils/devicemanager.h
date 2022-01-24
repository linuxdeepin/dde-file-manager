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
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "dfm-base/dfm_base_global.h"
#include "dbus_interface/devicemanagerdbus_interface.h"

#include <QObject>
#include <QPointer>
#include <QDBusServiceWatcher>

DFMBASE_BEGIN_NAMESPACE

struct NetworkMountInfo
{
    QString userName;
    QString domain;
    QString passwd;
    int saveMode { 0 };
    bool anonymous { false };

    inline bool isValid()
    {
        return anonymous || (!userName.isEmpty() && !domain.isEmpty() && !passwd.isEmpty());
    }
};

class DeviceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceManager)
public:
    static DeviceManager &instance();
    QPointer<DeviceManagerInterface> getDeviceInterface();
    bool connectToServer();
    void initConnection();

    // TODO(xust) if launch in root mode, invoke service directly.

    void invokeDetachAllMountedDevices();
    void invokeDetachAllMountedDevicesForced();
    bool invokeIsMonotorWorking();
    QStringList invokeBlockDevicesIdList(const QVariantMap &opt);
    QStringList invokeProtolcolDevicesIdList(const QVariantMap &opt);
    QVariantMap invokeQueryBlockDeviceInfo(const QString &id, bool detail = false);
    QVariantMap invokeQueryProtocolDeviceInfo(const QString &id, bool detail = false);
    void invokeDetachBlockDevice(const QString &id);
    void invokeDetachBlockDeviceForced(const QString &id);
    void invokeUnmountBlockDeviceForced(const QString &id);
    void invokeDetachProtocolDevice(const QString &id);
    void invokeRenameBlockDevice(const QString &id, const QString &newName);

    QString invokeUnlockBlockDevice(const QString &id, const QString &passwd);
    QString invokeMountBlockDevice(const QString &id);
    void invokeUnmountBlockDevice(const QString &id);
    QString invokeMountProtocolDevice(const QString &id);
    void invokeUnmountProtocolDevice(const QString &id);

    using HandleAfterUnlock = std::function<void(const QString &unlockResult)>;
    void unlockAndDo(const QString &id, const QString &passwd, HandleAfterUnlock handler);

    using AskForMountInfo = std::function<NetworkMountInfo(QString message)>;
    QString invokeMountNetworkDevice(const QString address, AskForMountInfo requestMountInfo);

signals:
    void serviceUnregistered(const QString &service);
    void serviceRegistered(const QString &service);
    void askStopScanning(const QString &method, const QString &id);

private:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();

private:
    QScopedPointer<DeviceManagerInterface> deviceInterface { nullptr };
    QScopedPointer<QDBusServiceWatcher> watcher { nullptr };
};

DFMBASE_END_NAMESPACE

#define DeviceManagerInstance DFMBASE_NAMESPACE::DeviceManager::instance()

#endif   // DEVICEMANAGER_H
