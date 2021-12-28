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
#include "coreeventreceiver.h"
#include "corehelper.h"

#include "dfm-base/base/urlroute.h"

#include <functional>

DPCORE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

CoreEventReceiver::CoreEventReceiver()
    : AutoEventHandlerRegister<CoreEventReceiver>()
{
    // add event topic map in here
    CoreEventReceiver::eventTopicHandlers = {
        { SideBar::EventTopic::kSideBarItem, std::bind(&CoreEventReceiver::handleSideBarItemTopic, this, std::placeholders::_1) }
    };

    // add event data map in here
    CoreEventReceiver::eventDataHandlers = {
        { SideBar::EventData::kCdAction, std::bind(&CoreEventReceiver::handleSideBarItemActived, this, std::placeholders::_1) }
    };
}

void CoreEventReceiver::eventProcess(const dpf::Event &event)
{
    QString topic { event.topic() };
    if (!eventTopicHandlers.contains(topic)) {
        qWarning() << "Invalid event topic: " << topic;
        return;
    }

    eventTopicHandlers[topic](event);
}

void CoreEventReceiver::handleSideBarItemTopic(const dpf::Event &event)
{
    QString subTopic { event.data().toString() };
    if (!eventDataHandlers.contains(subTopic)) {
        qWarning() << "Invalid event data: " << subTopic;
        return;
    }

    eventDataHandlers[subTopic](event);
}

void CoreEventReceiver::handleSideBarItemActived(const dpf::Event &event)
{
    QUrl url { qvariant_cast<QUrl>(event.property(SideBar::EventProperty::kUrl)) };
    if (!url.isValid()) {
        qWarning() << "Invalid Url: " << url;
        return;
    }

    if (url.scheme() == SchemeTypes::kFile) {
        quint64 windowId { qvariant_cast<quint64>(event.property(SideBar::EventProperty::kWindowId)) };
        CoreHelper::cd(windowId, url);
    }
}
