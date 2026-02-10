// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTDBUSUTILS_H
#define VAULTDBUSUTILS_H

#include "dfmplugin_vault_global.h"

#include <QObject>
#include <QVariant>
#include <QDBusConnection>

namespace dfmplugin_vault {
class VaultDBusUtils : public QObject
{
    Q_OBJECT
public:
    static VaultDBusUtils *instance();

    static QVariant vaultManagerDBusCall(QString function, const QVariant &value = {});

    static void lockEventTriggered(QObject *obj, const char *cslot = nullptr);

    static int getLeftoverErrorInputTimes();

    static void leftoverErrorInputTimesMinusOne();

    static void startTimerOfRestorePasswordInput();

    static int getNeedWaitMinutes();

    static void restoreNeedWaitMinutes();

    static void restoreLeftoverErrorInputTimes();
    static bool isServiceRegister(QDBusConnection::BusType type, const QString &serviceName);

    static bool isFullConnectInternet();

public Q_SLOTS:
    void handleChangedVaultState(const QVariantMap &map);
    void handleLockScreenDBus(const QDBusMessage &msg);

private:
    VaultDBusUtils();
};
}
#endif   // VAULTDBUSUTILS_H
