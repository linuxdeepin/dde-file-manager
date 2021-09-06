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

#ifndef EVENTCALLED_H
#define EVENTCALLED_H

#include "dfm-framework/event/event.h"
#include "dfm-framework/event/eventcallproxy.h"
#include "windowservice/containerkeys.h"

DSB_FM_USE_NAMESPACE

struct EventCalled
{
public:
    static void sendOpenNewWindowEvent(int windowIdx)
    {
        dpf::Event event;
        event.setTopic(EventKeys::TOPIC_WINDOW_EVENT);
        event.setData(EventKeys::DATA_OPEN_NEW_WINDOW);
        event.setProperty(EventKeys::PROPERTY_KEY_WINDOW_INDEX, windowIdx);
        dpf::EventCallProxy::callEvent(event);
    }
};

#endif // EVENTCALLED_H
