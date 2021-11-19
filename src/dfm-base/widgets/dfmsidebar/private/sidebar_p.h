/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef SIDEBAR_P_H
#define SIDEBAR_P_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE
class SideBar;
class GroupName;
class SideBarPrivate : public QObject
{
    Q_OBJECT
    friend class SideBar;
    SideBar *const q;
    SideBarView *sidebarView;
    SideBarModel *sidebarModel;
    explicit SideBarPrivate(SideBar *qq);
    void customContextMenuCall(const QPoint &pos);

private Q_SLOTS:
    void onItemClicked(const QModelIndex &index);
};

DFMBASE_END_NAMESPACE

#endif   // SIDEBAR_P_H
