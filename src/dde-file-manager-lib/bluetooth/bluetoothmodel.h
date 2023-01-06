// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHMODEL_H
#define BLUETOOTHMODEL_H

#include "bluetooth/bluetoothadapter.h"

#include <QObject>

/**
 * @brief 蓝牙模块数据模型，管理所有的适配器，是一对多关系
 */
class BluetoothModel : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothModel(QObject *parent = nullptr);

    /**
     * @brief 返回所有蓝牙 adapter_id - adapter
     * @return
     */
    QMap<QString, const BluetoothAdapter *> adapters() const;

    /**
     * @brief 通过 adapter_id_id 返回 adapter
     * @param id
     * @return
     */
    const BluetoothAdapter *adapterById(const QString &id);

public Q_SLOTS:
    void addAdapter(BluetoothAdapter *adapter);
    const BluetoothAdapter *removeAdapater(const QString &adapterId);

Q_SIGNALS:
    void adapterAdded(const BluetoothAdapter *adapter) const;
    void adapterRemoved(const BluetoothAdapter *adapter) const;

private:
    QMap<QString, const BluetoothAdapter *> m_adapters;

};

#endif // BLUETOOTHMODEL_H
