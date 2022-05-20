/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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
#ifndef WORKSPACEEVENTSEQUENCE_H
#define WORKSPACEEVENTSEQUENCE_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/dfm_global_defines.h"
#include <dfm-framework/framework.h>

#include <QAbstractItemView>

DPWORKSPACE_BEGIN_NAMESPACE

class WorkspaceEventSequence : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceEventSequence)

public:
    static WorkspaceEventSequence *instance();

    bool doPaintListItem(int role, const QUrl &url, QPainter *painter, QRectF *rect);
    bool doPaintIconItem(int role, const QUrl &url, QPainter *painter, QRectF *rect);
    bool doCheckDragTarget(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool doFetchSelectionModes(const QUrl &url, QList<QAbstractItemView::SelectionMode> *modes);
    bool doFetchCustomColumnRoles(const QUrl &rootUrl, QList<DFMGLOBAL_NAMESPACE::ItemRoles> *roleList);
    bool doFetchCustomRoleDiaplayName(const QUrl &rootUrl, const DFMGLOBAL_NAMESPACE::ItemRoles role, QString *displayName);
    bool doFetchCustomRoleData(const QUrl &rootUrl, const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role, QVariant *data);

private:
    explicit WorkspaceEventSequence(QObject *parent = nullptr);
    DPF_NAMESPACE::EventSequenceManager *sequence();
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEEVENTSEQUENCE_H
