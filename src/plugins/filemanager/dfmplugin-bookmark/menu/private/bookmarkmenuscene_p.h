/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef BOOKMARKMENUSCENE_P_H
#define BOOKMARKMENUSCENE_P_H

#include "dfmplugin_bookmark_global.h"

#include "interfaces/private/abstractmenuscene_p.h"

DPBOOKMARK_BEGIN_NAMESPACE

class BookmarkMenuScene;
class BookmarkMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class BookmarkMenuScene;

public:
    explicit BookmarkMenuScenePrivate(DFMBASE_NAMESPACE::AbstractMenuScene *qq);

    bool showBookMarkMenu = true;
};

DPBOOKMARK_END_NAMESPACE

#endif   // BOOKMARKMENUSCENE_P_H
