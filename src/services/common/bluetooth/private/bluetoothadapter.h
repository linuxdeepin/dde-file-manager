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

#ifndef BLUETOOTHADAPTER_H
#define BLUETOOTHADAPTER_H

#include "dfm_common_service_global.h"

#include "bluetoothdevice.h"

#include <QObject>
#include <QMap>

/**
 * @brief 蓝牙模块适配器，管理终端的蓝牙设备，是一对多的关系
 */
DSC_BEGIN_NAMESPACE
class BluetoothAdapter : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothAdapter(QObject *parent = nullptr);

    inline QString getId() const { return id; }
    void setId(const QString &id);

    inline QString getName() const { return name; }
    void setName(const QString &name);

    inline bool isPowered() const { return powered; }
    void setPowered(bool powered);

    QMap<QString, const BluetoothDevice *> getDevices() const;
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
    QString id;
    QString name;
    bool powered;
    QMap<QString, const BluetoothDevice *> devices;
};
DSC_END_NAMESPACE

#endif   // BLUETOOTHADAPTER_H
