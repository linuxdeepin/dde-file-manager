// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEPROXYMANAGER_P_H
#define DEVICEPROXYMANAGER_P_H

#include "devicemanager_interface.h"

#include <dfm-base/dfm_base_global.h>

#include <QScopedPointer>
#include <QList>
#include <QtCore/qobjectdefs.h>
#include <QReadWriteLock>

using DeviceManagerInterface = OrgDeepinFilemanagerServerDeviceManagerInterface;
class QDBusServiceWatcher;
namespace dfmbase {

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
    void initMounts();

    void connectToDBus();
    void connectToAPI();
    void disconnCurrentConnections();

private Q_SLOTS:
    void addMounts(const QString &id, const QString &mpt);
    void removeMounts(const QString &id);

private:
    DeviceProxyManager *q { nullptr };
    QScopedPointer<DeviceManagerInterface> devMngDBus;
    QScopedPointer<QDBusServiceWatcher> dbusWatcher;
    QList<QMetaObject::Connection> connections;
    int currentConnectionType = kNoneConnection;   // 0 for API connection and 1 for DBus connection
    QReadWriteLock lock;
    QMap<QString, QString> externalMounts;
    QMap<QString, QString> allMounts;

    enum {
        kNoneConnection = -1,
        kAPIConnecting,
        kDBusConnecting
    };
};

}

#endif   // DEVICEPROXYMANAGER_P_H
