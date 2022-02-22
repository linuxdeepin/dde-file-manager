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

#include "bluetoothmodel.h"

DSC_USE_NAMESPACE

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
