/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "windowservice/contexts.h"

DPF_EVENT_HANDLER(RecentEventReceiver,DSB_FM_NAMESPACE::EventTypes::TOPIC_WINDOW_EVENT);

void RecentEventReceiver::eventProcess(const dpf::Event &event)
{
    if (event.topic() == EventTypes::TOPIC_WINDOW_EVENT)
        windowEvent(event);
}

void RecentEventReceiver::windowEvent(const dpf::Event &event)
{
    if (event.data() == EventTypes::DATA_OPEN_NEW_WINDOW)
    {
        WindowService* windowService = dpf::PluginServiceContext::service<WindowService>("WindowService");
        qCCritical(RecentPlugin) << Q_FUNC_INFO << windowService;
        if (windowService) {
            //do new sidebar and plugin all menu;
            int winIdx = event.property(EventTypes::PROPERTY_KEY_WINDOW_INDEX).toInt();
            qCCritical(RecentPlugin) << "recver:"
                                     << EventTypes::PROPERTY_KEY_WINDOW_INDEX
                                     << winIdx;

            QIcon recentIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::RecentPath));

            auto recentItem = new SideBarItem(recentIcon,
                                              RecentUtil::sidebarDisplayText,
                                              "core", RecentUtil::onlyRootUrl());

            recentItem->setFlags(recentItem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

            windowService->insertSideBarItem(winIdx, 0, recentItem);
        }
    }
}
