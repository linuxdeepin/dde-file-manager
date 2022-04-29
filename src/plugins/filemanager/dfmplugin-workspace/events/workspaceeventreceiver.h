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
#include "dfm-base/dfm_global_defines.h"

#include <QObject>
#include <QUrl>
#include <QDir>
#include <QAbstractItemView>

DPWORKSPACE_BEGIN_NAMESPACE

class WorkspaceEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceEventReceiver)

public:
    static WorkspaceEventReceiver *instance();

    void initConnection();

public slots:
    void handleTileBarSwitchModeTriggered(quint64 windowId, DFMBASE_NAMESPACE::Global::ViewMode mode);
    void handleOpenNewTabTriggered(quint64 windowId, const QUrl &url);
    void handleShowCustomTopWidget(quint64 windowId, const QString &cheme, bool visible);
    void handleCloseTabs(const QUrl &url);
    void handleSelectFiles(quint64 windowId, const QList<QUrl> &files);

    void handleSetSelectionMode(const quint64 windowId, const QAbstractItemView::SelectionMode mode);
    void handleSetEnabledSelectionModes(const quint64 windowId, const QList<QAbstractItemView::SelectionMode> &modes);

    void handleSetViewDragEnabled(const quint64 windowId, const bool enabled);
    void handleSetViewDragDropMode(const quint64 windowId, const QAbstractItemView::DragDropMode mode);
    void handleClosePersistentEditor(const quint64 windowId, const QModelIndex &index);

    void handleSetViewFilter(const quint64 windowId, const QDir::Filters &filters);
    void handleSetNameFilter(const quint64 windowId, const QStringList &filters);
    void handleSetReadOnly(const quint64 windowId, const bool readOnly);

private:
    explicit WorkspaceEventReceiver(QObject *parent = nullptr);
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEEVENTRECEIVER_H
