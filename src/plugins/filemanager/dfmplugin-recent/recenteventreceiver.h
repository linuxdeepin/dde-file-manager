/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef RECENTEVENTRECEIVER_H
#define RECENTEVENTRECEIVER_H

#include "recentlog.h"
#include "recentutil.h"

#include "windowservice.h"
#include "window/contexts.h" // TODO(zhangs): hide

#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE

class RecentEventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<RecentEventReceiver>
{
    Q_OBJECT

public:
    static EventHandler::Type type()
    {
        return EventHandler::Type::Sync;
    }

    static QStringList topics()
    {
         return QStringList() << DSB_FM_NAMESPACE::EventTypes::TOPIC_WINDOW_EVENT;
    }

    explicit RecentEventReceiver() : AutoEventHandlerRegister<RecentEventReceiver>() {}
    void eventProcess(const dpf::Event &event) override;
    void windowEvent(const dpf::Event &event);
};

#endif // RECENTEVENTRECVER_H
