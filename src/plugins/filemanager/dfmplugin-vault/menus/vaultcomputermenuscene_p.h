// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCOMPUTERMENUSCENE_P_H
#define VAULTCOMPUTERMENUSCENE_P_H

#include "dfmplugin_vault_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

class QAction;

namespace dfmplugin_vault {

class VaultComputerMenuScene;
class VaultComputerMenuScenePrivate : DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class VaultComputerMenuScene;
    QList<QAction *> acts;

public:
    explicit VaultComputerMenuScenePrivate(VaultComputerMenuScene *qq);
};

}

#endif   // VAULTCOMPUTERMENUSCENE_P_H
