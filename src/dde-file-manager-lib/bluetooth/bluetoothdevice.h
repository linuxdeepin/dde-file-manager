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

#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include <QObject>

/**
 * @brief 蓝牙连接的终端设备，例如含有蓝牙模块的手机/电脑
 */
class BluetoothDevice : public QObject
{
    Q_OBJECT

public:
    enum State {
        StateUnavailable = 0, // 未连接
        StateAvailable   = 1, // 可链接
        StateConnected   = 2  // 完成连接
    };
    Q_ENUM(State)

    explicit BluetoothDevice(QObject *parent = nullptr);

    inline QString id() const { return m_id; }
    void setId(const QString &id);

    inline QString name() const { return m_name; }
    void setName(const QString &name);

    inline QString alias() const { return m_alias; }
    void setAlias(const QString &alias);

    inline QString icon() const { return m_icon; }
    void setIcon(const QString &icon);

    inline bool paired() const { return m_paired; }
    void setPaired(bool paired);

    inline bool trusted() const { return m_trusted; }
    void setTrusted(bool trusted);

    inline State state() const { return m_state; }
    void setState(const State &state);

Q_SIGNALS:
    void nameChanged(const QString &name) const;
    void aliasChanged(const QString &alias) const;
    void pairedChanged(const bool &paired) const;
    void stateChanged(const State &state) const;
    void trustedChanged(const bool trusted) const;

private:
    QString m_id;
    QString m_name;
    QString m_alias;
    QString m_icon; // phone/computer
    bool m_paired;  // 是否配对
    bool m_trusted; // 是否信任该设备
    State m_state;
};

#endif // BLUETOOTHDEVICE_H
