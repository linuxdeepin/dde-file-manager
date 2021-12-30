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
#ifndef WORKSPACEEVENTRECEIVER_H
#define WORKSPACEEVENTRECEIVER_H

#include "dfmplugin_workspace_global.h"
#include "services/filemanager/titlebar/titlebar_defines.h"

#include <dfm-framework/event/eventhandler.h>
#include <dfm-framework/event/eventcallproxy.h>

#include <QMap>
#include <QString>

DPWORKSPACE_BEGIN_NAMESPACE
class WorkspaceEventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<WorkspaceEventReceiver>
{
    Q_OBJECT

public:
    using HandlerMap = QMap<QString, std::function<void(const dpf::Event &)>>;

    static EventHandler::Type type()
    {
        return EventHandler::Type::Sync;
    }

    static QStringList topics()
    {
        return QStringList() << DSB_FM_NAMESPACE::TitleBar::EventTopic::kTitleBar;
    }

    explicit WorkspaceEventReceiver();
    void eventProcess(const dpf::Event &event) override;

private:   // event topics
    void handleTitleBarTopic(const dpf::Event &event);
    void callHandler(const dpf::Event &event, const HandlerMap &map);

private:   // event data (sub topics)
    void handleTileBarSwitchModeTriggered(const dpf::Event &event);

private:
    HandlerMap eventTopicHandlers;
};
DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEEVENTRECEIVER_H
