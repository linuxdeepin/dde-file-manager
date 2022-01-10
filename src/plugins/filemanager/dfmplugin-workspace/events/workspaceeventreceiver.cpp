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

WorkspaceEventReceiver::WorkspaceEventReceiver(QObject *parent)
    : QObject(parent)
{
}

WorkspaceEventReceiver *WorkspaceEventReceiver::instance()
{
    static WorkspaceEventReceiver receiver;
    return &receiver;
}

void WorkspaceEventReceiver::handleTileBarSwitchModeTriggered(quint64 windowId, TitleBar::ViewMode mode)
{
    WorkspaceHelper::instance()->switchViewMode(windowId, mode);
}

void WorkspaceEventReceiver::handleOpenNewTabTriggered(quint64 windowId, const QUrl &url)
{
    WorkspaceHelper::instance()->openUrlInNewTab(windowId, url);
}
