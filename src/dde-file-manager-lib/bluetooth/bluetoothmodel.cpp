// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
