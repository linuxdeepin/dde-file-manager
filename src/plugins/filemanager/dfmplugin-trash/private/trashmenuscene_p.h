// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHMENUSCENE_P_H
#define TRASHMENUSCENE_P_H

#include "dfmplugin_trash_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>
#include <dfm-base/dfm_global_defines.h>

namespace dfmplugin_trash {

class TrashMenuScene;
class TrashMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class TrashMenuScene;

public:
    explicit TrashMenuScenePrivate(TrashMenuScene *qq);

    void updateMenu(QMenu *menu);
    void updateSortSubMenu(QMenu *menu);
    void updateGroupSubMenu(QMenu *menu);
    void groupByRole(int role);

private:
    /**
     * @brief Generic method to update submenus with custom actions
     * @param menu The submenu to update
     * @param actionsToRemove List of action IDs to remove from the submenu
     * @param actionsToAdd List of action IDs to add to the submenu (in order)
     * @param currentRoleSlot Slot name to query current role
     * @param roleToActionMap Mapping from ItemRoles to action IDs for state update
     */
    void updateSubMenuGeneric(QMenu *menu, 
                             const QStringList &actionsToRemove,
                             const QStringList &actionsToAdd,
                             const QString &currentRoleSlot,
                             const QMap<DFMBASE_NAMESPACE::Global::ItemRoles, QString> &roleToActionMap);

private:
    TrashMenuScene *q;

    QMultiHash<QString, QString> selectSupportActions;
};

}

#endif   // TRASHMENUSCENE_P_H
