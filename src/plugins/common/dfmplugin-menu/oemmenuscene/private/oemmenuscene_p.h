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

#include "interfaces/private/abstractmenuscene_p.h"

DPMENU_BEGIN_NAMESPACE

class OemMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
public:
    friend class FileOperatorMenuScene;
    explicit OemMenuScenePrivate(OemMenuScene *qq);

    QList<QAction *> childActions(QAction *action);

    QList<QAction *> oemActions;
    QList<QAction *> oemChildActions;
};

DPMENU_END_NAMESPACE

#endif   // OEMMENUSCENE_P_H
