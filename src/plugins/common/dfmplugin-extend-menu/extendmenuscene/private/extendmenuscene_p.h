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
#ifndef EXTENDMENUSCENE_P_H
#define EXTENDMENUSCENE_P_H

#include "extendmenuscene/extendmenuscene.h"
#include "extendmenuscene/extendmenu/dcustomactiondefine.h"

#include "interfaces/private/abstractmenuscene_p.h"

DPEXTENDMENU_BEGIN_NAMESPACE

class ExtendMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
public:
    friend class FileOperatorMenuScene;
    explicit ExtendMenuScenePrivate(ExtendMenuScene *qq);

    QList<QAction *> extendActions;

    QMap<int, QList<QAction *>> cacheLocateActions;
    QMap<QAction *, DCustomActionDefines::Separator> cacheActionsSeparator;
};

DPEXTENDMENU_END_NAMESPACE

#endif   // EXTENDMENUSCENE_P_H
