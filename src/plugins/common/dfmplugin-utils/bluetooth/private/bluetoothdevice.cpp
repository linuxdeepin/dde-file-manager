// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetoothdevice.h"

using namespace dfmplugin_utils;

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
