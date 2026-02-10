// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTMENUSCENE_P_H
#define VAULTMENUSCENE_P_H

#include "vaultmenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_vault {
class VaultMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
public:
    friend class VaultMenuScene;
    explicit VaultMenuScenePrivate(VaultMenuScene *qq = nullptr);

    QStringList emptyMenuActionRule();

    QStringList normalMenuActionRule();

    void filterMenuAction(QMenu *menu, const QStringList &actions);

private:
    void updateMenu(QMenu *menu);
};
}
#endif   // VAULTMENUSCENE_P_H
