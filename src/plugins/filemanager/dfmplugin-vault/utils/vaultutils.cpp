// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultutils.h"
#include "vaultdefine.h"

using namespace dfmplugin_vault;
using namespace PolkitQt1;

VaultUtils &VaultUtils::instance()
{
    static VaultUtils ins;
    return ins;
}

void VaultUtils::showAuthorityDialog(const QString &actionId)
{
    fmDebug() << "Vault: Showing authority dialog for action:" << actionId;
    auto ins = Authority::instance();
    ins->checkAuthorization(actionId,
                            UnixProcessSubject(getpid()),
                            Authority::AllowUserInteraction);
    connect(ins, &Authority::checkAuthorizationFinished,
            this, &VaultUtils::slotCheckAuthorizationFinished);
}

void VaultUtils::slotCheckAuthorizationFinished(Authority::Result result)
{
    fmDebug() << "Vault: Authorization check finished with result:" << static_cast<int>(result);
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultUtils::slotCheckAuthorizationFinished);

    if (Authority::Yes != result) {
        fmWarning() << "Vault: Authorization denied";
        emit resultOfAuthority(false);
    } else {
        fmDebug() << "Vault: Authorization granted";
        emit resultOfAuthority(true);
    }
}

VaultUtils::VaultUtils(QObject *parent)
    : QObject (parent)
{
}

