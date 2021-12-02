/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "recenteventreceiver.h"
#include "window/contexts.h"   // TODO(zhangs): hide

#include <dfm-framework/framework.h>

#include <QMenu>

void RecentEventReceiver::eventProcess(const dpf::Event &event)
{
    QString eventTopic = event.topic();
    if (eventTopic == EventTypes::kTopicWindowEvent) {
        windowEvent(event);
    } else if (eventTopic == EventTypes::kSidebarContextMenuEvent) {
        // 筛选事件
        if (event.property(EventTypes::kPropertySidebarItemUrl).toUrl().scheme() == RecentUtil::recentScheme)
            sidebarContextMenuEvent(event);
    }
}

void RecentEventReceiver::windowEvent(const dpf::Event &event)
{
    if (event.data() == EventTypes::kDataOpenNewWindow) {
        auto &ctx = dpfInstance.serviceContext();
        WindowService *windowService = ctx.service<WindowService>(WindowService::name());
        dpfDebug() << Q_FUNC_INFO << windowService;
        if (windowService) {
            //do new sidebar and plugin all menu;
            quint64 winIdx = event.property(EventTypes::kPropertyKeyWindowIndex).toULongLong();
            dpfDebug() << "recver:"
                       << EventTypes::kPropertyKeyWindowIndex
                       << winIdx;

            QIcon recentIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::RecentPath));

            auto recentItem = new SideBarItem(recentIcon,
                                              RecentUtil::sidebarDisplayText,
                                              "core", RecentUtil::onlyRootUrl());

            recentItem->setFlags(recentItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

            windowService->insertSideBarItem(winIdx, 0, recentItem);
        }
    }
}

void RecentEventReceiver::sidebarContextMenuEvent(const dpf::Event &event)
{
    if (event.data() == EventTypes::kDataSidebarContextMenu) {
        // 创建右键菜单
        QMenu *menu = new QMenu();
        QAction *openInNewWindow = new QAction(tr("Open in new window"), menu);
        QAction *openInNewTab = new QAction(tr("Open in new tab"), menu);
        QAction *clearRecentHistory = new QAction(tr("Clear recent history"), menu);
        menu->addAction(openInNewWindow);
        menu->addAction(openInNewTab);
        menu->addSeparator();
        menu->addAction(clearRecentHistory);

        // 获取显示位置
        QPoint pos = event.property(EventTypes::kPropertySidebarItemPos).toPoint();
        menu->exec(pos);
        delete menu;
        menu = nullptr;
    }
}
