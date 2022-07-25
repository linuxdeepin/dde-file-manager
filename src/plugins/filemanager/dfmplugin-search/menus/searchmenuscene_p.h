/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SEARCHMENUSCENE_P_H
#define SEARCHMENUSCENE_P_H

#include "dfmplugin_search_global.h"

#include "interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_search {

class SearchMenuScene;
class SearchMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class SearchMenuScene;

public:
    explicit SearchMenuScenePrivate(SearchMenuScene *qq);

private:
    void updateMenu(QMenu *menu);
    bool openFileLocation(const QString &path);
    void disableSubScene(DFMBASE_NAMESPACE::AbstractMenuScene *scene, const QString &sceneName);

private:
    SearchMenuScene *q;
};

}
#endif   // SEARCHMENUSCENE_P_H
