/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#pragma once

#include "dfmsidebariteminterface.h"

#define SIDEBAR_ID_BOOKMARK "__bookmark"

class DUrl;

DFM_BEGIN_NAMESPACE

class DFMSideBarBookmarkItemHandler : public DFMSideBarItemInterface
{
public:
    static DFMSideBarItem * createItem(const DUrl &url);

    DFMSideBarBookmarkItemHandler();

    void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    QMenu * contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    void rename(const DFMSideBarItem *item, QString name) override;
};

DFM_END_NAMESPACE
