/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef DEVICEPROXYMANAGER_P_H
#define DEVICEPROXYMANAGER_P_H

#include "dfm-base/dfm_base_global.h"

#include <QScopedPointer>
#include <QList>
#include <qt5/QtCore/qobjectdefs.h>

class DeviceManagerInterface;
class QDBusServiceWatcher;
DFMBASE_BEGIN_NAMESPACE

class DeviceProxyManager;
class DeviceProxyManagerPrivate : public QObject
{
    friend class DeviceProxyManager;
    Q_OBJECT

public:
    explicit DeviceProxyManagerPrivate(DeviceProxyManager *qq, QObject *parent = nullptr);
    ~DeviceProxyManagerPrivate();

    bool isDBusRuning();
    void initConnection();
    void initExternalMounts();

    void connectToDBus();
    void connectToAPI();
    void disconnCurrentConnections();

private Q_SLOTS:
    void addExternalMounts(const QString &id, const QString &mpt);
    void removeExternalMounts(const QString &id);

private:
    DeviceProxyManager *q { nullptr };
    QScopedPointer<DeviceManagerInterface> devMngDBus;
    QScopedPointer<QDBusServiceWatcher> dbusWatcher;
    QList<QMetaObject::Connection> connections;
    int currentConnectionType = kNoneConnection;   // 0 for API connection and 1 for DBus connection
    QMap<QString, QString> externalMounts;

    enum {
        kNoneConnection = -1,
        kAPIConnecting,
        kDBusConnecting
    };
};

DFMBASE_END_NAMESPACE

#endif   // DEVICEPROXYMANAGER_P_H
