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
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>
#include <functional>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
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

void WorkspaceEventReceiver::initConnection()
{
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kSwitchViewMode,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTileBarSwitchModeTriggered);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenNewTab,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleOpenNewTabTriggered);

    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kShowCustomTopWidget,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleShowCustomTopWidget);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kCloseTabs,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleCloseTabs);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSelectFiles,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSelectFiles);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSelectAll,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSelectAll);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSetSelectionMode,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetSelectionMode);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSetEnabledSelectionModes,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetEnabledSelectionModes);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSetViewDragEnabled,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewDragEnabled);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSetViewDragDropMode,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewDragDropMode);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSetViewFilter,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewFilter);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSetNameFilter,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetNameFilter);
    dpfInstance.eventDispatcher().subscribe(Workspace::EventType::kSetReadOnly,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetReadOnly);
}

void WorkspaceEventReceiver::handleTileBarSwitchModeTriggered(quint64 windowId, DFMBASE_NAMESPACE::Global::ViewMode mode)
{
    WorkspaceHelper::instance()->switchViewMode(windowId, int(mode));
}

void WorkspaceEventReceiver::handleOpenNewTabTriggered(quint64 windowId, const QUrl &url)
{
    WorkspaceHelper::instance()->openUrlInNewTab(windowId, url);
}

void WorkspaceEventReceiver::handleShowCustomTopWidget(quint64 windowId, const QString &scheme, bool visible)
{
    WorkspaceHelper::instance()->setCustomTopWidgetVisible(windowId, scheme, visible);
}

void WorkspaceEventReceiver::handleCloseTabs(const QUrl &url)
{
    WorkspaceHelper::instance()->closeTab(url);
}

void WorkspaceEventReceiver::handleSelectFiles(quint64 windowId, const QList<QUrl> &files)
{
    WorkspaceHelper::instance()->selectFiles(windowId, files);
}

void WorkspaceEventReceiver::handleSelectAll(quint64 windowId)
{
    WorkspaceHelper::instance()->selectAll(windowId);
}

void WorkspaceEventReceiver::handleSetSelectionMode(const quint64 windowId, const QAbstractItemView::SelectionMode mode)
{
    WorkspaceHelper::instance()->setSelectionMode(windowId, mode);
}

void WorkspaceEventReceiver::handleSetEnabledSelectionModes(const quint64 windowId, const QList<QAbstractItemView::SelectionMode> &modes)
{
    WorkspaceHelper::instance()->setEnabledSelectionModes(windowId, modes);
}

void WorkspaceEventReceiver::handleSetViewDragEnabled(const quint64 windowId, const bool enabled)
{
    WorkspaceHelper::instance()->setViewDragEnabled(windowId, enabled);
}

void WorkspaceEventReceiver::handleSetViewDragDropMode(const quint64 windowId, const QAbstractItemView::DragDropMode mode)
{
    WorkspaceHelper::instance()->setViewDragDropMode(windowId, mode);
}

void WorkspaceEventReceiver::handleClosePersistentEditor(const quint64 windowId, const QModelIndex &index)
{
    WorkspaceHelper::instance()->closePersistentEditor(windowId, index);
}

void WorkspaceEventReceiver::handleSetViewFilter(const quint64 windowId, const QDir::Filters &filters)
{
    WorkspaceHelper::instance()->setViewFilter(windowId, filters);
}

void WorkspaceEventReceiver::handleSetNameFilter(const quint64 windowId, const QStringList &filters)
{
    WorkspaceHelper::instance()->setNameFilter(windowId, filters);
}

void WorkspaceEventReceiver::handleSetReadOnly(const quint64 windowId, const bool readOnly)
{
    WorkspaceHelper::instance()->setReadOnly(windowId, readOnly);
}
