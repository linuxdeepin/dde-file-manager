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

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPWORKSPACE_NAMESPACE) };

WorkspaceEventReceiver::WorkspaceEventReceiver(QObject *parent)
    : QObject(parent)
{
}

WorkspaceEventReceiver::~WorkspaceEventReceiver()
{
    dpfSignalDispatcher->unsubscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged",
                                     WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged);
}

WorkspaceEventReceiver *WorkspaceEventReceiver::instance()
{
    static WorkspaceEventReceiver receiver;
    return &receiver;
}

void WorkspaceEventReceiver::initConnection()
{
    // signal event
    dpfSignalDispatcher->subscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged",
                                   WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged);

    // slot event
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_CloseTab",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleCloseTabs);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_FileUpdate",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleFileUpdate);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_SetViewFilter",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewFilter);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_SetNameFilter",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetNameFilter);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_CurrentSortRole",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleCurrentSortRole);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_SetSort",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetSort);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_SelectAll",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSelectAll);

    dpfSignalDispatcher->subscribe(GlobalEventType::kSwitchViewMode,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTileBarSwitchModeTriggered);

    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewTab,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleOpenNewTabTriggered);

    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFileResult,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handlePasteFileResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyResult,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handlePasteFileResult);

    dpfSignalDispatcher->subscribe(Workspace::EventType::kShowCustomTopWidget,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleShowCustomTopWidget);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kSelectFiles,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSelectFiles);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kSetSelectionMode,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetSelectionMode);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kSetEnabledSelectionModes,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetEnabledSelectionModes);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kSetViewDragEnabled,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewDragEnabled);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kSetViewDragDropMode,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewDragDropMode);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kSetViewFilter,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewFilter);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kSetReadOnly,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetReadOnly);
}

void WorkspaceEventReceiver::handleTileBarSwitchModeTriggered(quint64 windowId, int mode)
{
    WorkspaceHelper::instance()->switchViewMode(windowId, mode);
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

void WorkspaceEventReceiver::handleSetSort(quint64 windowId, ItemRoles role)
{
    WorkspaceHelper::instance()->setSort(windowId, role);
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

void WorkspaceEventReceiver::handlePasteFileResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(srcUrls)
    Q_UNUSED(errMsg)

    if (!destUrls.isEmpty())
        WorkspaceHelper::instance()->laterRequestSelectFiles(destUrls);
}

void WorkspaceEventReceiver::handleFileUpdate(const QUrl &url)
{
    WorkspaceHelper::instance()->fileUpdate(url);
}

ItemRoles WorkspaceEventReceiver::handleCurrentSortRole(quint64 windowId)
{
    return WorkspaceHelper::instance()->sortRole(windowId);
}
