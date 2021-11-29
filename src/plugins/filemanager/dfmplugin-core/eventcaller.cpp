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
#include "eventcaller.h"

#include <dfm-framework/framework.h>

void EventCaller::sendOpenNewWindowEvent(quint64 windowIdx)
{
    dpf::Event event;
    event.setTopic(EventTypes::kTopicWindowEvent);
    event.setData(EventTypes::kDataOpenNewWindow);
    event.setProperty(EventTypes::kPropertyKeyWindowIndex, windowIdx);
    dpfInstance.eventProxy().pubEvent(event);
}

void EventCaller::sendSideBarContextMenuEvent(const QUrl &url, const QPoint &pos)
{
    dpf::Event event;
    event.setTopic(EventTypes::kSidebarContextMenuEvent);
    event.setData(EventTypes::kDataSidebarContextMenu);
    event.setProperty(EventTypes::kPropertySidebarItemUrl, QVariant(url));
    event.setProperty(EventTypes::kPropertySidebarItemPos, QVariant(pos));
    dpfInstance.eventProxy().pubEvent(event);
}

void EventCaller::sendSearchEvent(const QUrl &targetUrl, const QString &keyword, quint64 winIdx)
{
    dpf::Event event;
    event.setTopic(EventTypes::kTopicSearchEvent);
    event.setData(EventTypes::kDataSearch);
    event.setProperty(EventTypes::kPropertyTargetUrl, QVariant(targetUrl));
    event.setProperty(EventTypes::kPropertyKeyword, QVariant(keyword));
    event.setProperty(EventTypes::kPropertyKeyWindowIndex, winIdx);
    dpfInstance.eventProxy().pubEvent(event);
}
