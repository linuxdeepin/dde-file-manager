// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHMENUSCENE_P_H
#define SEARCHMENUSCENE_P_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_search {

class SearchMenuScene;
class SearchMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class SearchMenuScene;

public:
    explicit SearchMenuScenePrivate(SearchMenuScene *qq);

private:
    void createAction(QMenu *menu, const QString &actName, bool isSubAct = false, bool checkable = false);
    void updateMenu(QMenu *menu);
    void updateSortMenu(QMenu *menu);
    void updateGroupSubMenu(QMenu *menu);
    void groupByRole(const QString &strategy);
    bool openFileLocation(const QString &path);
    void disableSubScene(DFMBASE_NAMESPACE::AbstractMenuScene *scene, const QString &sceneName);

private:
    SearchMenuScene *q;
    QStringList emptyWhitelist;
};

}
#endif   // SEARCHMENUSCENE_P_H
