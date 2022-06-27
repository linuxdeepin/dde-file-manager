/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef NEWCREATEMENUSCENE_P_H
#define NEWCREATEMENUSCENE_P_H
#include "menuscene/newcreatemenuscene.h"

#include "interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class NewCreateMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class NewCreateMenuScene;
    explicit NewCreateMenuScenePrivate(NewCreateMenuScene *qq);
};

}

#endif   // NEWCREATEMENUSCENE_P_H
