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

#pragma once

#include "dfmsidebariteminterface.h"
#include "dtkwidget_global.h"

#define SIDEBAR_ID_DEVICE "__device"

DWIDGET_BEGIN_NAMESPACE
class DViewItemAction;
DWIDGET_END_NAMESPACE

class DUrl;
class DFMSideBarDeviceItemHandler : public DFMSideBarItemInterface
{
public:
    static DTK_WIDGET_NAMESPACE::DViewItemAction * createUnmountOrEjectAction(const DUrl &url);
    static DFMLeftSideBarItem * createItem(const DUrl &url);

    DFMSideBarDeviceItemHandler(QObject *parent = nullptr);

    void cdAction(const DFMLeftSideBar *sidebar, const DFMLeftSideBarItem* item) override;
    QMenu * contextMenu(const DFMLeftSideBar *sidebar, const DFMLeftSideBarItem* item) override;
};
