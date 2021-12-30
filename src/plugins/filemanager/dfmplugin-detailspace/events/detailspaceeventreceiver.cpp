/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "detailspaceeventreceiver.h"
#include "utils/detailspacehelper.h"

#include <functional>

DPDETAILSPACE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

DetailSpaceEventReceiver::DetailSpaceEventReceiver()
{
    // add event topic map in here
    DetailSpaceEventReceiver::eventTopicHandlers = {
        { TitleBar::EventTopic::kTitleBar, std::bind(&DetailSpaceEventReceiver::handleTitleBarTopic, this, std::placeholders::_1) }
    };
}

void DetailSpaceEventReceiver::eventProcess(const dpf::Event &event)
{
    QString topic { event.topic() };
    if (!eventTopicHandlers.contains(topic)) {
        qWarning() << "Invalid event topic: " << topic;
        return;
    }

    eventTopicHandlers[topic](event);
}

void DetailSpaceEventReceiver::handleTitleBarTopic(const dpf::Event &event)
{
    static HandlerMap eventDataHandlers = {
        { TitleBar::EventData::kShowDetailView, std::bind(&DetailSpaceEventReceiver::handleTileBarShowDetailView, this, std::placeholders::_1) }
    };

    callHandler(event, eventDataHandlers);
}

void DetailSpaceEventReceiver::callHandler(const dpf::Event &event, const DetailSpaceEventReceiver::HandlerMap &map)
{
    QString subTopic { event.data().toString() };
    if (!map.contains(subTopic)) {
        qWarning() << "Invalid event data: " << subTopic;
        return;
    }

    map[subTopic](event);
}

void DetailSpaceEventReceiver::handleTileBarShowDetailView(const dpf::Event &event)
{
    quint64 windowId { qvariant_cast<quint64>(event.property(TitleBar::EventProperty::kWindowId)) };
    bool checked { qvariant_cast<bool>(event.property(TitleBar::EventProperty::kDetailState)) };
    DetailSpaceHelper::showDetailView(windowId, checked);
}
