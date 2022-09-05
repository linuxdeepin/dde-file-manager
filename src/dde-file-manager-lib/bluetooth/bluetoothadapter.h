// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
