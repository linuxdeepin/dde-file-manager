/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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

BluetoothDevice::BluetoothDevice(QObject *parent) : QObject(parent)
{

}

void BluetoothDevice::setId(const QString &id)
{
    m_id = id;
}

void BluetoothDevice::setName(const QString &name)
{
    m_name = name;
}

void BluetoothDevice::setAlias(const QString &alias)
{
    m_alias = alias;
}

void BluetoothDevice::setIcon(const QString &icon)
{
    m_icon = icon;
}

void BluetoothDevice::setPaired(bool paired)
{
    m_paired = paired;
}

void BluetoothDevice::setTrusted(bool trusted)
{
    m_trusted = trusted;
}

void BluetoothDevice::setConnecting(bool connecting)
{
    m_connecting = connecting;
}

void BluetoothDevice::setState(const BluetoothDevice::State &state)
{
    m_state = state;
}
