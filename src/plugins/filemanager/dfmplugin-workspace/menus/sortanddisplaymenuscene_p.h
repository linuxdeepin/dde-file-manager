/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SORTANDDISPLAYMENUSCENE_P_H
#define SORTANDDISPLAYMENUSCENE_P_H

#include "dfmplugin_workspace_global.h"
#include <interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

class FileView;
class SortAndDisplayMenuScene;
class SortAndDisplayMenuScenePrivate : public AbstractMenuScenePrivate
{
    friend SortAndDisplayMenuScene;

public:
    explicit SortAndDisplayMenuScenePrivate(AbstractMenuScene *qq);

private:
    void createEmptyMenu(QMenu *parent);
    QMenu *addSortByActions(QMenu *menu);
    QMenu *addDisplayAsActions(QMenu *menu);

    void sortByRole(int role);
    void updateEmptyAreaActionState();

private:
    FileView *view = nullptr;
};

DPWORKSPACE_END_NAMESPACE

#endif   // SORTANDDISPLAYMENUSCENE_P_H
