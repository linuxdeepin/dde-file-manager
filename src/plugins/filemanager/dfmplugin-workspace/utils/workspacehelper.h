/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef DETAILSPACEHELPER_H
#define DETAILSPACEHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QMap>
#include <QMutex>
#include <QObject>

DPWORKSPACE_BEGIN_NAMESPACE

class WorkspaceWidget;
class WorkspaceHelper : public QObject
{
    Q_OBJECT
public:
    static WorkspaceHelper *instance();
    WorkspaceWidget *findWorkspaceByWindowId(quint64 windowId);
    void addWorkspace(quint64 windowId, WorkspaceWidget *workspace);
    void removeWorkspace(quint64 windowId);
    quint64 windowId(QWidget *sender);
    void switchViewMode(quint64 windowId, int viewMode);
signals:
    void viewModeChanged(quint64 windowId, int viewMode);

private:
    explicit WorkspaceHelper(QObject *parent = nullptr);
    static QMutex &mutex();
    static QMap<quint64, WorkspaceWidget *> kWorkspaceMap;
    Q_DISABLE_COPY(WorkspaceHelper)
};

DPWORKSPACE_END_NAMESPACE

#endif   // DETAILSPACEHELPER_H
