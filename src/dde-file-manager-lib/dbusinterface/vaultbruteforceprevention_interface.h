/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
