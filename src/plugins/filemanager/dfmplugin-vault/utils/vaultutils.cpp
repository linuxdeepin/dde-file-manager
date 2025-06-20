// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
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
    auto ins = Authority::instance();
    ins->checkAuthorization(actionId,
                            UnixProcessSubject(getpid()),
                            Authority::AllowUserInteraction);
    connect(ins, &Authority::checkAuthorizationFinished,
            this, &VaultUtils::slotCheckAuthorizationFinished);
}

void VaultUtils::slotCheckAuthorizationFinished(Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultUtils::slotCheckAuthorizationFinished);

    if (Authority::Yes != result)
        emit resultOfAuthority(false);
    else
        emit resultOfAuthority(true);
}

VaultUtils::VaultUtils(QObject *parent)
    : QObject (parent)
{
}

