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
#include "workspaceeventsequence.h"

#include "services/filemanager/workspace/workspace_defines.h"

#include <QPainter>
#include <QRectF>

Q_DECLARE_METATYPE(QRectF *)
Q_DECLARE_METATYPE(QPainter *)
Q_DECLARE_METATYPE(Qt::DropAction *)

DPF_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

WorkspaceEventSequence *WorkspaceEventSequence::instance()
{
    static WorkspaceEventSequence ins;
    return &ins;
}

bool WorkspaceEventSequence::doPaintListItem(int role, const QUrl &url, QPainter *painter, QRectF *rect)
{
    return sequence()->run(Workspace::EventType::kPaintListItem, role, url, painter, rect);
}

bool WorkspaceEventSequence::doPaintIconItem(int role, const QUrl &url, QPainter *painter, QRectF *rect)
{
    return sequence()->run(Workspace::EventType::kPaintIconItem, role, url, painter, rect);
}

bool WorkspaceEventSequence::doCheckDragTarget(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    return sequence()->run(Workspace::EventType::kCheckDragDropAction, urls, urlTo, action);
}

WorkspaceEventSequence::WorkspaceEventSequence(QObject *parent)
    : QObject(parent)
{
}

EventSequenceManager *WorkspaceEventSequence::sequence()
{
    return &dpfInstance.eventSequence();
}
