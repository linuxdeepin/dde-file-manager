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

#include "bluetoothadapter.h"

BluetoothAdapter::BluetoothAdapter(QObject *parent)
    : QObject(parent),
      m_id(""),
      m_name(""),
      m_powered(false)
{

}

void BluetoothAdapter::setId(const QString &id)
{
    m_id = id;
}

void BluetoothAdapter::setName(const QString &name)
{
    if (name != m_name) {
        m_name = name;
        Q_EMIT nameChanged(name);
    }
}

void BluetoothAdapter::setPowered(bool powered)
{
    if (powered != m_powered) {
        m_powered = powered;
        Q_EMIT poweredChanged(powered);
    }
}

QMap<QString, const BluetoothDevice *> BluetoothAdapter::devices() const
{
    return m_devices;
}

const BluetoothDevice *BluetoothAdapter::deviceById(const QString &id) const
{
    return m_devices.keys().contains(id) ? m_devices[id] : nullptr;
}

void BluetoothAdapter::addDevice(const BluetoothDevice *device)
{
    if (!deviceById(device->id())) {
        m_devices[device->id()] = device;
        Q_EMIT deviceAdded(device);
    }
}

void BluetoothAdapter::removeDevice(const QString &deviceId)
{
    const BluetoothDevice *device = nullptr;

    device = deviceById(deviceId);
    if (device) {
        m_devices.remove(deviceId);
        Q_EMIT deviceRemoved(deviceId);
    }
}
