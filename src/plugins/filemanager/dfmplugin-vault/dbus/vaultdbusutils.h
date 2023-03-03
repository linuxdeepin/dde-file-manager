// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTDBUSUTILS_H
#define VAULTDBUSUTILS_H

#include "dfmplugin_vault_global.h"

#include <QObject>
#include <QVariant>

namespace dfmplugin_vault {
class VaultDBusUtils
{
public:
    static QVariant vaultManagerDBusCall(QString function, const QVariant &vaule = {});

    static int getVaultPolicy();

    static bool setVaultPolicyState(int policyState);

    static void lockEventTriggered(QObject *obj, const char *cslot = nullptr);

    static int getLeftoverErrorInputTimes();

    static void leftoverErrorInputTimesMinusOne();

    static void startTimerOfRestorePasswordInput();

    static int getNeedWaitMinutes();

    static void restoreNeedWaitMinutes();

    static void restoreLeftoverErrorInputTimes();
};
}
#endif   // VAULTDBUSUTILS_H
