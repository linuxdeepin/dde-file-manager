// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetoothadapter.h"

using namespace dfmplugin_utils;

BluetoothAdapter::BluetoothAdapter(QObject *parent)
    : QObject(parent),
      id(""),
      name(""),
      powered(false)
{
}

void BluetoothAdapter::setId(const QString &id)
{
    this->id = id;
}

void BluetoothAdapter::setName(const QString &name)
{
    if (this->name != name) {
        this->name = name;
        Q_EMIT nameChanged(name);
    }
}

void BluetoothAdapter::setPowered(bool powered)
{
    if (this->powered != powered) {
        this->powered = powered;
        Q_EMIT poweredChanged(powered);
    }
}

QMap<QString, const BluetoothDevice *> BluetoothAdapter::getDevices() const
{
    return devices;
}

const BluetoothDevice *BluetoothAdapter::deviceById(const QString &id) const
{
    return devices.keys().contains(id) ? devices[id] : nullptr;
}

void BluetoothAdapter::addDevice(const BluetoothDevice *device)
{
    if (!deviceById(device->getId())) {
        devices[device->getId()] = device;
        Q_EMIT deviceAdded(device);
    }
}

void BluetoothAdapter::removeDevice(const QString &deviceId)
{
    const BluetoothDevice *device = nullptr;

    device = deviceById(deviceId);
    if (device) {
        devices.remove(deviceId);
        Q_EMIT deviceRemoved(deviceId);
    }
}
