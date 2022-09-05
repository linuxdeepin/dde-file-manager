// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
