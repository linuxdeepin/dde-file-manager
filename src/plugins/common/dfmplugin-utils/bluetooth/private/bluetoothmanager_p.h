// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHMANAGER_P_H
#define BLUETOOTHMANAGER_P_H

#include "dfmplugin_utils_global.h"
#include "bluetoothmanager.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QMap>

template<class T>
class QFutureWatcher;

namespace dfmplugin_utils {
class BluetoothModel;
class BluetoothDevice;
class BluetoothManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothManagerPrivate(BluetoothManager *qq);
    ~BluetoothManagerPrivate();
    BluetoothManagerPrivate(BluetoothManagerPrivate &) = delete;
    BluetoothManagerPrivate &operator=(BluetoothManagerPrivate &) = delete;

    void resolve(const QDBusReply<QString> &req);
    void initInterface();
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
    QMap<QString, bool> longFilenameFailures;

    Q_DECLARE_PUBLIC(BluetoothManager)
};
}

#endif   // BLUETOOTHMANAGER_P_H
