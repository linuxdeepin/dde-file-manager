/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "workspaceeventreceiver.h"
#include "utils/workspacehelper.h"

#include "dfm-base/base/urlroute.h"

#include <functional>

DPWORKSPACE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

WorkspaceEventReceiver::WorkspaceEventReceiver()
    : AutoEventHandlerRegister<WorkspaceEventReceiver>()
{
    // add event topic map in here
    WorkspaceEventReceiver::eventTopicHandlers = {
        { TitleBar::EventTopic::kTitleBar, std::bind(&WorkspaceEventReceiver::handleTitleBarTopic, this, std::placeholders::_1) }
    };
}

void WorkspaceEventReceiver::eventProcess(const dpf::Event &event)
{
    QString topic { event.topic() };
    if (!eventTopicHandlers.contains(topic)) {
        qWarning() << "Invalid event topic: " << topic;
        return;
    }

    eventTopicHandlers[topic](event);
}

void WorkspaceEventReceiver::handleTitleBarTopic(const dpf::Event &event)
{
    // add event data map in here
    static HandlerMap eventDataHandlers = {
        { TitleBar::EventData::kSwitchMode, std::bind(&WorkspaceEventReceiver::handleTileBarSwitchModeTriggered, this, std::placeholders::_1) }
    };

    callHandler(event, eventDataHandlers);
}

void WorkspaceEventReceiver::callHandler(const dpf::Event &event, const HandlerMap &map)
{
    QString subTopic { event.data().toString() };
    if (!map.contains(subTopic)) {
        qWarning() << "Invalid event data: " << subTopic;
        return;
    }

    map[subTopic](event);
}

void WorkspaceEventReceiver::handleTileBarSwitchModeTriggered(const dpf::Event &event)
{

    quint64 windowId { qvariant_cast<quint64>(event.property(TitleBar::EventProperty::kWindowId)) };
    int viewMode { qvariant_cast<int>(event.property(TitleBar::EventProperty::kViewMode).toInt()) };
    WorkspaceHelper::instance()->switchViewMode(windowId, viewMode);
}
