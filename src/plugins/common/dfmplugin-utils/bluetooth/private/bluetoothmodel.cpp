// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetoothmodel.h"

using namespace dfmplugin_utils;

BluetoothModel::BluetoothModel(QObject *parent)
    : QObject(parent)
{
    adapters.clear();
}

/**
 * @brief 返回所有蓝牙 adapter_id - adapter
 * @return
 */
QMap<QString, const BluetoothAdapter *> BluetoothModel::getAdapters() const
{
    return adapters;
}

/**
 * @brief 通过 adapter_id_id 返回 adapter
 * @param id
 * @return
 */
const BluetoothAdapter *BluetoothModel::adapterById(const QString &id)
{
    return adapters.keys().contains(id) ? adapters[id] : nullptr;
}

void BluetoothModel::addAdapter(BluetoothAdapter *adapter)
{
    if (!adapterById(adapter->getId())) {
        adapters[adapter->getId()] = adapter;
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
        adapters.remove(adapterId);
        Q_EMIT adapterRemoved(adapter);
    }

    return adapter;
}
