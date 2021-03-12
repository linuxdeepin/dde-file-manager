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

#include "bluetoothmodel.h"

BluetoothModel::BluetoothModel(QObject *parent) : QObject(parent)
{
    m_adapters.clear();
}

QMap<QString, const BluetoothAdapter *> BluetoothModel::adapters() const
{
    return m_adapters;
}

const BluetoothAdapter *BluetoothModel::adapterById(const QString &id)
{
    return m_adapters.keys().contains(id) ? m_adapters[id] : nullptr;
}

void BluetoothModel::addAdapter(BluetoothAdapter *adapter)
{
    if (!adapterById(adapter->id())) {
        m_adapters[adapter->id()] = adapter;
        Q_EMIT adapterAdded(adapter);
        return;
    }
    adapter->deleteLater();
}

const BluetoothAdapter *BluetoothModel::removeAdapater(const QString &adapterId)
{
    const BluetoothAdapter *adapter = nullptr;

    adapter = adapterById(adapterId);
    if (adapter) {
        m_adapters.remove(adapterId);
        Q_EMIT adapterRemoved(adapter);
    }

    return adapter;
}
