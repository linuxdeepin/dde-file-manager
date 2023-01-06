// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
