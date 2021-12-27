/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef SIDEBARMODEL_P_CPP
#define SIDEBARMODEL_P_CPP

#include "dfmplugin_sidebar_global.h"

#include <QObject>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarModel;
class SideBarModelPrivate : public QObject
{
    Q_OBJECT
    friend class SideBarModel;
    SideBarModel *const q;
    explicit SideBarModelPrivate(SideBarModel *qq);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARMODEL_P_CPP
