/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef OEMMENUSCENE_P_H
#define OEMMENUSCENE_P_H

#include "oemmenuscene/oemmenuscene.h"
#include "oemmenuscene/oemmenu.h"

#include "interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_menu {

class OemMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
public:
    explicit OemMenuScenePrivate(OemMenuScene *qq);

    QList<QAction *> childActions(QAction *action);

public:
    OemMenu *oemMenu = nullptr;

    QList<QAction *> oemActions;
    QList<QAction *> oemChildActions;
};

}

#endif   // OEMMENUSCENE_P_H
