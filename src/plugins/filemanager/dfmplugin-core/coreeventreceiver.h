/*
* Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef COREEVENTRECEIVER_H
#define COREEVENTRECEIVER_H

#include "contexts.h"

#include <dfm-framework/event/eventhandler.h>
#include <dfm-framework/event/eventcallproxy.h>

class CoreEventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<CoreEventReceiver>
{
    Q_OBJECT

public:
    static EventHandler::Type type()
    {
        return EventHandler::Type::Sync;
    }

    static QStringList topics()
    {
        return QStringList() << DSB_FM_NAMESPACE::EventTypes::kSidebarContextMenuEvent
                             << DSB_FM_NAMESPACE::EventTypes::kTopicWindowEvent;
    }

    explicit CoreEventReceiver()
        : AutoEventHandlerRegister<CoreEventReceiver>() {}
    void eventProcess(const dpf::Event &event) override;
    void sidebarContextMenuEvent(const dpf::Event &event);
    void setRootUrlEvent(const dpf::Event &event);
};

#endif   // COREEVENTRECEIVER_H
