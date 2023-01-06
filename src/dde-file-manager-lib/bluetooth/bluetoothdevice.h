// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
