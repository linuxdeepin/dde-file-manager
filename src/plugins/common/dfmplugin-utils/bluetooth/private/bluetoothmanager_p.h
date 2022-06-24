/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             lvwujun<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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

#ifndef BLUETOOTHMANAGER_P_H
#define BLUETOOTHMANAGER_P_H

#include "dfmplugin_utils_global.h"
#include "bluetoothmanager.h"

#include <QDBusConnection>
#include <QDBusReply>

template<class T>
class QFutureWatcher;

DPUTILS_BEGIN_NAMESPACE
class BluetoothModel;
class BluetoothDevice;
class BluetoothManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothManagerPrivate(BluetoothManager *qq);
    BluetoothManagerPrivate(BluetoothManagerPrivate &) = delete;
    BluetoothManagerPrivate &operator=(BluetoothManagerPrivate &) = delete;

    void resolve(const QDBusReply<QString> &req);
    void init();
    void initConnects();
    bool connectBluetoothDBusSignals(const QString &signal, const char *slot);
    void inflateAdapter(BluetoothAdapter *adapter, const QJsonObject &adapterObj);
    void inflateDevice(BluetoothDevice *device, const QJsonObject &deviceObj);
    QDBusPendingCall getBluetoothDevices(const QDBusObjectPath &adapter);
    QDBusPendingCall getBluetoothAdapters();
    QDBusPendingReply<QDBusObjectPath> sendFiles(const QString &device, const QStringList &files);
    void cancelTransferSession(const QDBusObjectPath &sessionPath);

public Q_SLOTS:
    void onServiceValidChanged(bool valid);
    void onAdapterAdded(const QString &json);
    void onAdapterRemoved(const QString &json);
    void onAdapterPropertiesChanged(const QString &json);
    void onDeviceAdded(const QString &json);
    void onDeviceRemoved(const QString &json);
    void onDevicePropertiesChanged(const QString &json);
    void onTransferCreated(const QString &file, const QDBusObjectPath &transferPath, const QDBusObjectPath &sessionPath);
    void onTransferRemoved(const QString &file, const QDBusObjectPath &transferPath, const QDBusObjectPath &sessionPath, bool done);
    void onObexSessionCreated(const QDBusObjectPath &sessionPath);
    void onObexSessionRemoved(const QDBusObjectPath &sessionPath);
    void onObexSessionProgress(const QDBusObjectPath &sessionPath, qulonglong totalSize, qulonglong transferred, int currentIndex);
    void onTransferFailed(const QString &file, const QDBusObjectPath &sessionPath, const QString &errInfo);

public:
    BluetoothManager *q_ptr { nullptr };
    BluetoothModel *model { nullptr };
    QDBusInterface *bluetoothInter { nullptr };
    QFutureWatcher<QPair<QString, QString>> *watcher { nullptr };

    Q_DECLARE_PUBLIC(BluetoothManager)
};
DPUTILS_END_NAMESPACE

#endif   // BLUETOOTHMANAGER_P_H
