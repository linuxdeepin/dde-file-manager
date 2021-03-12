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

#ifndef BLUETOOTHADAPTER_H
#define BLUETOOTHADAPTER_H

#include "bluetooth/bluetoothdevice.h"

#include <QObject>
#include <QMap>

/**
 * @brief 蓝牙模块适配器，管理终端的蓝牙设备，是一对多的关系
 */
class BluetoothAdapter : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothAdapter(QObject *parent = nullptr);

    inline QString id() const { return m_id; }
    void setId(const QString &id);

    inline QString name() const { return m_name; }
    void setName(const QString &name);

    inline bool powered() const { return m_powered; }
    void setPowered(bool powered);

    QMap<QString, const BluetoothDevice *> devices() const;
    const BluetoothDevice *deviceById(const QString &id) const;

public Q_SLOTS:
    void addDevice(const BluetoothDevice *device);
    void removeDevice(const QString &deviceId);

Q_SIGNALS:
    void deviceAdded(const BluetoothDevice *device) const;
    void deviceRemoved(const QString &deviceId) const;
    void nameChanged(const QString &name) const;
    void poweredChanged(const bool &powered) const;

private:
    QString m_id;
    QString m_name;
    bool m_powered;
    QMap<QString, const BluetoothDevice *> m_devices;
};

#endif // BLUETOOTHADAPTER_H
