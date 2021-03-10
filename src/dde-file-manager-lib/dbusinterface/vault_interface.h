/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#ifndef VAULT_INTERFACE_H
#define VAULT_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.filemanager.daemon.VaultManager
 */
class VaultInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.filemanager.daemon.VaultManager"; }

public:
    VaultInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~VaultInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<bool> checkAuthentication(const QString &type)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(type);
        return asyncCallWithArgumentList(QStringLiteral("checkAuthentication"), argumentList);
    }

    inline QDBusPendingReply<> clearLockEvent()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("clearLockEvent"), argumentList);
    }

    inline QDBusPendingReply<qulonglong> getLastestTime()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("getLastestTime"), argumentList);
    }

    inline QDBusPendingReply<qulonglong> getSelfTime()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("getSelfTime"), argumentList);
    }

    inline QDBusPendingReply<bool> isLockEventTriggered()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("isLockEventTriggered"), argumentList);
    }

    inline QDBusPendingReply<> setRefreshTime(qulonglong time)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(time);
        return asyncCallWithArgumentList(QStringLiteral("setRefreshTime"), argumentList);
    }

    inline QDBusPendingReply<> sysUserChanged(const QString &curUser)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(curUser);
        return asyncCallWithArgumentList(QStringLiteral("sysUserChanged"), argumentList);
    }

    inline QDBusPendingReply<> triggerLockEvent()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("triggerLockEvent"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
  namespace deepin {
    namespace filemanager {
      namespace daemon {
        typedef ::VaultInterface VaultManager;
      }
    }
  }
}
#endif
