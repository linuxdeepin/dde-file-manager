/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
