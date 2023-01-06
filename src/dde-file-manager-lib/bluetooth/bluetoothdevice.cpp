// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetoothdevice.h"

BluetoothDevice::BluetoothDevice(QObject *parent) : QObject(parent),m_paired(false),m_trusted(false),m_state(StateUnavailable)
{

}

void BluetoothDevice::setId(const QString &id)
{
    m_id = id;
}

void BluetoothDevice::setName(const QString &name)
{
    if (name != m_name) {
        m_name = name;
        Q_EMIT nameChanged(name);
    }
}

void BluetoothDevice::setAlias(const QString &alias)
{
    if (alias != m_alias) {
        m_alias = alias;
        Q_EMIT aliasChanged(alias);
    }
}

void BluetoothDevice::setIcon(const QString &icon)
{
    m_icon = icon;
}

void BluetoothDevice::setPaired(bool paired)
{
    if (paired != m_paired) {
        m_paired = paired;
        Q_EMIT pairedChanged(paired);
    }
}

void BluetoothDevice::setTrusted(bool trusted)
{
    if (trusted != m_trusted) {
        m_trusted = trusted;
        Q_EMIT trustedChanged(trusted);
    }
}

void BluetoothDevice::setState(const BluetoothDevice::State &state)
{
    if (state != m_state) {
        m_state = state;
        Q_EMIT stateChanged(state);
    }
}
