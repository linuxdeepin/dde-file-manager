/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef PLUGINSIDECAR_H
#define PLUGINSIDECAR_H

#include <QObject>
#include <QPointer>

class DeviceManagerInterface;

class PluginSidecar: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginSidecar)
public:
    static PluginSidecar &instance();
    QPointer<DeviceManagerInterface> getDeviceInterface();

    bool connectToServer();
    void invokeEjectAllDevices();
    bool invokeIsMonotorWorking();
    QStringList invokeBlockDevicesIdList();
    QStringList invokeProtolcolDevicesIdList();
    QString invokeQueryBlockDeviceInfo(const QString &id);
    QString invokeQueryProtocolDeviceInfo(const QString &id);
    void invokeEjectDevice(const QString &id);

private:
    explicit PluginSidecar(QObject *parent = nullptr);
    ~PluginSidecar();

private:
    QSharedPointer<DeviceManagerInterface> deviceInterface {nullptr};
};

#define SidecarInstance PluginSidecar::instance()

#endif // PLUGINSIDECAR_H
