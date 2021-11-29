/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "coreeventreceiver.h"
#include "dfm-base/base/urlroute.h"

#include <QMenu>

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void CoreEventReceiver::eventProcess(const dpf::Event &event)
{
    QString eventTopic = event.topic();
    if (eventTopic == EventTypes::kSidebarContextMenuEvent) {
        // 事件筛选
        QString scheme = event.property(EventTypes::kPropertySidebarItemUrl).toUrl().scheme();
        if (scheme == SchemeTypes::kHome
            || scheme == SchemeTypes::kDesktop
            || scheme == SchemeTypes::kVideos
            || scheme == SchemeTypes::kMusic
            || scheme == SchemeTypes::kPictures
            || scheme == SchemeTypes::kDocuments
            || scheme == SchemeTypes::kDownloads)
            sidebarContextMenuEvent(event);
    }
}

void CoreEventReceiver::sidebarContextMenuEvent(const dpf::Event &event)
{
    if (event.data() == EventTypes::kDataSidebarContextMenu) {
        // 创建右键菜单
        QMenu *menu = new QMenu();
        QAction *openInNewWindow = new QAction(tr("Open in new window"), menu);
        QAction *openInNewTab = new QAction(tr("Open in new tab"), menu);
        QAction *property = new QAction(tr("Properties"), menu);
        menu->addAction(openInNewWindow);
        menu->addAction(openInNewTab);
        menu->addSeparator();
        menu->addAction(property);

        // 获取显示位置
        QPoint pos = event.property(EventTypes::kPropertySidebarItemPos).toPoint();
        menu->exec(pos);
        delete menu;
        menu = nullptr;
    }
}
