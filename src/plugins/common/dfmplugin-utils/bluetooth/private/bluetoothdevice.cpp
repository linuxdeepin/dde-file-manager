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

#include "bluetoothdevice.h"

DPUTILS_USE_NAMESPACE

BluetoothDevice::BluetoothDevice(QObject *parent)
    : QObject(parent), paired(false), trusted(false), state(kStateUnavailable)
{
}

void BluetoothDevice::setId(const QString &id)
{
    this->id = id;
}

void BluetoothDevice::setName(const QString &name)
{
    if (this->name != name) {
        this->name = name;
        Q_EMIT nameChanged(name);
    }
}

void BluetoothDevice::setAlias(const QString &alias)
{
    if (this->alias != alias) {
        this->alias = alias;
        Q_EMIT aliasChanged(alias);
    }
}

void BluetoothDevice::setIcon(const QString &icon)
{
    this->icon = icon;
}

void BluetoothDevice::setPaired(bool paired)
{
    if (this->paired != paired) {
        this->paired = paired;
        Q_EMIT pairedChanged(paired);
    }
}

void BluetoothDevice::setTrusted(bool trusted)
{
    if (this->trusted != trusted) {
        this->trusted = trusted;
        Q_EMIT trustedChanged(trusted);
    }
}

void BluetoothDevice::setState(const BluetoothDevice::State &state)
{
    if (this->state != state) {
        this->state = state;
        Q_EMIT stateChanged(state);
    }
}
