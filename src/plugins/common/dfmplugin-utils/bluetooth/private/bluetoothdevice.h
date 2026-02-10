// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include "dfmplugin_utils_global.h"

#include <QObject>

/**
 * @brief 蓝牙连接的终端设备，例如含有蓝牙模块的手机/电脑
 */
namespace dfmplugin_utils {

class BluetoothDevice : public QObject
{
    Q_OBJECT

public:
    enum State {
        kStateUnavailable = 0,   // 未连接
        kStateAvailable = 1,   // 可链接
        kStateConnected = 2   // 完成连接
    };
    Q_ENUM(State)

    explicit BluetoothDevice(QObject *parent = nullptr);

    inline QString getId() const { return id; }
    void setId(const QString &id);

    inline QString getName() const { return name; }
    void setName(const QString &name);

    inline QString getAlias() const { return alias; }
    void setAlias(const QString &alias);

    inline QString getIcon() const { return icon; }
    void setIcon(const QString &icon);

    inline bool isPaired() const { return paired; }
    void setPaired(bool paired);

    inline bool isTrusted() const { return trusted; }
    void setTrusted(bool trusted);

    inline State getState() const { return state; }
    void setState(const State &state);

Q_SIGNALS:
    void nameChanged(const QString &name) const;
    void aliasChanged(const QString &alias) const;
    void pairedChanged(const bool &paired) const;
    void stateChanged(const State &state) const;
    void trustedChanged(const bool trusted) const;

private:
    QString id;
    QString name;
    QString alias;
    QString icon;   // phone/computer
    bool paired;   // 是否配对
    bool trusted;   // 是否信任该设备
    State state;
};
}
#endif   // BLUETOOTHDEVICE_H
