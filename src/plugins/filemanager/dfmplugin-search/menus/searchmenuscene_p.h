// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    void updateMenu(QMenu *menu);
    bool openFileLocation(const QString &path);
    void disableSubScene(DFMBASE_NAMESPACE::AbstractMenuScene *scene, const QString &sceneName);
    void updateSubMenu(QMenu *menu);
    void updateSubMenuNormal(QMenu *menu);
    void updateSubMenuTrash(QMenu *menu);
    void updateSubMenuRecent(QMenu *menu);

private:
    SearchMenuScene *q;
};

}
#endif   // SEARCHMENUSCENE_P_H
