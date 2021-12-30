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
#include "workspacehelper.h"
#include "services/filemanager/windows/windowsservice.h"

#include <dfm-framework/framework.h>

DPWORKSPACE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

QMap<quint64, WorkspaceWidget *> WorkspaceHelper::kWorkspaceMap {};

WorkspaceHelper *WorkspaceHelper::instance()
{
    static WorkspaceHelper helper;
    return &helper;
}

WorkspaceWidget *WorkspaceHelper::findWorkspaceByWindowId(quint64 windowId)
{
    if (!kWorkspaceMap.contains(windowId))
        return nullptr;

    return kWorkspaceMap[windowId];
}

void WorkspaceHelper::addWorkspace(quint64 windowId, WorkspaceWidget *workspace)
{
    QMutexLocker locker(&WorkspaceHelper::mutex());
    if (!kWorkspaceMap.contains(windowId))
        kWorkspaceMap.insert(windowId, workspace);
}

void WorkspaceHelper::removeWorkspace(quint64 windowId)
{
    QMutexLocker locker(&WorkspaceHelper::mutex());
    if (kWorkspaceMap.contains(windowId))
        kWorkspaceMap.remove(windowId);
}

quint64 WorkspaceHelper::windowId(QWidget *sender)
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    return windowService->findWindowId(sender);
}

void WorkspaceHelper::switchViewMode(quint64 windowId, int viewMode)
{
    emit viewModeChanged(windowId, viewMode);
}

WorkspaceHelper::WorkspaceHelper(QObject *parent)
    : QObject(parent)
{
}

QMutex &WorkspaceHelper::mutex()
{
    static QMutex m;
    return m;
}
