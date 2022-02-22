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

#ifndef BLUETOOTHMODEL_H
#define BLUETOOTHMODEL_H

#include "bluetoothadapter.h"

#include <QObject>

/**
 * @brief 蓝牙模块数据模型，管理所有的适配器，是一对多关系
 */
DSC_BEGIN_NAMESPACE

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
DSC_END_NAMESPACE

#endif   // BLUETOOTHMODEL_H
