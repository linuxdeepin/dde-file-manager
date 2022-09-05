// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHMANAGER_P_H
#define BLUETOOTHMANAGER_P_H

#include "bluetoothmanager.h"

#include <QDBusConnection>

#include <com_deepin_daemon_bluetooth.h>
#include <com_deepin_dde_controlcenter.h>


using DBusBluetooth = com::deepin::daemon::Bluetooth;
using DBusControlcenter = com::deepin::dde::ControlCenter;

template<class T>
class QFutureWatcher;

class BluetoothManagerPrivate
{
public:
    explicit BluetoothManagerPrivate(BluetoothManager *qq);

    BluetoothManagerPrivate(BluetoothManagerPrivate &) = delete;
    BluetoothManagerPrivate &operator=(BluetoothManagerPrivate &) = delete;
    /**
     * @brief 解析蓝牙设备, 获取适配器和设备信息
     * @param req
     */
    void resolve(const QDBusReply<QString> &req);

    /**
     * @brief 蓝牙 dbus 信号的处理
     */
    void initConnects();

    /**
     * @brief 获取适配器信息
     * @param adapter
     * @param adapterObj
     */
    void inflateAdapter(BluetoothAdapter *adapter, const QJsonObject &adapterObj);

    /**
     * @brief 获取设备信息
     * @param device
     * @param deviceObj
     */
    void inflateDevice(BluetoothDevice *device, const QJsonObject &deviceObj);

public:
    BluetoothManager *q_ptr {nullptr};
    BluetoothModel *m_model {nullptr};
    DBusBluetooth *m_bluetoothInter {nullptr};
    DBusControlcenter *m_controlcenterInter {nullptr};
    QFutureWatcher<QPair<QString, QString>> *m_watcher {nullptr};

    Q_DECLARE_PUBLIC(BluetoothManager)
};

#endif // BLUETOOTHMANAGER_P_H
