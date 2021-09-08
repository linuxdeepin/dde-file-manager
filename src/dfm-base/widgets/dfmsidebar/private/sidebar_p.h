/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include "dfm_base_global.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE
class SideBar;
class GroupName;
class SideBarPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(SideBar)
    SideBar *const q_ptr;
    SideBarView *sidebarView;
    SideBarModel *sidebarModel;
    explicit SideBarPrivate(SideBar *qq);

private Q_SLOTS:
    void onItemActivated(const QModelIndex &index);
};

DFMBASE_END_NAMESPACE

#endif // SIDEBAR_P_H
