// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHMODEL_H
#define BLUETOOTHMODEL_H

#include "bluetoothadapter.h"

#include <QObject>

/**
 * @brief 蓝牙模块数据模型，管理所有的适配器，是一对多关系
 */
namespace dfmplugin_utils {

class BluetoothModel : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothModel(QObject *parent = nullptr);
    QMap<QString, const BluetoothAdapter *> getAdapters() const;
    const BluetoothAdapter *adapterById(const QString &id);

public Q_SLOTS:
    void addAdapter(BluetoothAdapter *adapter);
    const BluetoothAdapter *removeAdapater(const QString &adapterId);

Q_SIGNALS:
    void adapterAdded(const BluetoothAdapter *adapter) const;
    void adapterRemoved(const BluetoothAdapter *adapter) const;

private:
    QMap<QString, const BluetoothAdapter *> adapters;
};
}

#endif   // BLUETOOTHMODEL_H
