/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef EXTENDCANVASSCENE_P_H
#define EXTENDCANVASSCENE_P_H

#include "ddplugin_organizer_global.h"

#include "extendcanvasscene.h"
#include "organizer_defines.h"

#include "interfaces/private/abstractmenuscene_p.h"

DFMBASE_USE_NAMESPACE
DDP_ORGANIZER_BEGIN_NAMESPACE

class ExtendCanvasScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    explicit ExtendCanvasScenePrivate(ExtendCanvasScene *qq);
    void emptyMenu(QMenu *parent);
    void normalMenu(QMenu *parent);

    void updateEmptyMenu(QMenu *parent);
    void updateNormalMenu(QMenu *parent);

    QMenu *organizeBySubActions(QMenu *menu);

    static QString classifierToActionID(Classifier cf);
public:
    bool turnOn = false;
private:
    ExtendCanvasScene *q;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // EXTENDCANVASSCENE_P_H
