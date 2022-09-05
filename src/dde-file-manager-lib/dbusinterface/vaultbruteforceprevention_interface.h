// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTBRUTEFORCEPREVENTION_INTERFACE_H
#define VAULTBRUTEFORCEPREVENTION_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.filemanager.daemon.VaultManager2
 */
class VaultBruteForcePreventionInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.filemanager.daemon.VaultManager2"; }

public:
    VaultBruteForcePreventionInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~VaultBruteForcePreventionInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<int> getLeftoverErrorInputTimes(int userID)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(userID);
        return asyncCallWithArgumentList(QStringLiteral("getLeftoverErrorInputTimes"), argumentList);
    }

    inline QDBusPendingReply<int> getNeedWaitMinutes(int userID)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(userID);
        return asyncCallWithArgumentList(QStringLiteral("getNeedWaitMinutes"), argumentList);
    }

    inline QDBusPendingReply<> leftoverErrorInputTimesMinusOne(int userID)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(userID);
        return asyncCallWithArgumentList(QStringLiteral("leftoverErrorInputTimesMinusOne"), argumentList);
    }

    inline QDBusPendingReply<> restoreLeftoverErrorInputTimes(int userID)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(userID);
        return asyncCallWithArgumentList(QStringLiteral("restoreLeftoverErrorInputTimes"), argumentList);
    }

    inline QDBusPendingReply<> restoreNeedWaitMinutes(int userID)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(userID);
        return asyncCallWithArgumentList(QStringLiteral("restoreNeedWaitMinutes"), argumentList);
    }

    inline QDBusPendingReply<> startTimerOfRestorePasswordInput(int userID)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(userID);
        return asyncCallWithArgumentList(QStringLiteral("startTimerOfRestorePasswordInput"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
  namespace deepin {
    namespace filemanager {
      namespace daemon {
        typedef ::VaultBruteForcePreventionInterface VaultManager2;
      }
    }
  }
}
#endif
