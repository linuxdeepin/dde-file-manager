// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTMENUSCENE_P_H
#define RECENTMENUSCENE_P_H

#include "dfmplugin_recent_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>
#include <dfm-base/dfm_global_defines.h>

namespace dfmplugin_recent {

class RecentMenuScene;
class RecentMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class RecentMenuScene;

public:
    explicit RecentMenuScenePrivate(RecentMenuScene *qq);

    void updateMenu(QMenu *menu);
    void updateSortSubMenu(QMenu *menu);
    void updateGroupSubMenu(QMenu *menu);
    void disableSubScene(DFMBASE_NAMESPACE::AbstractMenuScene *scene, const QString &sceneName);
    void groupByRole(const QString &strategy);

private:
    /**
     * @brief Generic method to update submenus with custom actions
     * @param menu The submenu to update
     * @param actionsToRemove List of action IDs to remove from the submenu
     * @param actionsToAdd List of action IDs to add to the submenu (in order)
     */
    void updateSubMenuGeneric(QMenu *menu,
                              const QStringList &actionsToRemove,
                              const QStringList &actionsToAdd);

private:
    RecentMenuScene *q;

    QMultiHash<QString, QString> selectDisableActions;
    QMultiHash<QString, QString> emptyDisableActions;
};

}

#endif   // RECENTMENUSCENE_P_H
