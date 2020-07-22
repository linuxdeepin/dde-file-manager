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
#ifndef BLUETOOTHMODEL_H
#define BLUETOOTHMODEL_H

#include "bluetooth/bluetoothadapter.h"

#include <QObject>

class BluetoothModel : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothModel(QObject *parent = nullptr);

    void clear();

    QMap<QString, const BluetoothAdapter *> adapters() const;

    const BluetoothAdapter *adapterById(const QString &id);

public Q_SLOTS:
    void addAdapter(BluetoothAdapter *adapter);

Q_SIGNALS:
    void adapterAdded(const BluetoothAdapter *adapter) const;

private:
    QMap<QString, const BluetoothAdapter *> m_adapters;

};

#endif // BLUETOOTHMODEL_H
