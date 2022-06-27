/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#ifndef RECENTMENUSCENE_P_H
#define RECENTMENUSCENE_P_H

#include "dfmplugin_recent_global.h"

#include "dfm-base/interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_recent {

class RecentMenuScene;
class RecentMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class RecentMenuScene;

public:
    explicit RecentMenuScenePrivate(RecentMenuScene *qq);

    void updateMenu(QMenu *menu);
    void updateSubMenu(QMenu *menu);
    void disableSubScene(DFMBASE_NAMESPACE::AbstractMenuScene *scene, const QString &sceneName);

private:
    RecentMenuScene *q;

    QMultiHash<QString, QString> selectDisableActions;
    QMultiHash<QString, QString> emptyDisableActions;
};

}

#endif   // RECENTMENUSCENE_P_H
