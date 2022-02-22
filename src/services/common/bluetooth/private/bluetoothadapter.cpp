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

#include "bluetoothadapter.h"

DSC_USE_NAMESPACE
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
