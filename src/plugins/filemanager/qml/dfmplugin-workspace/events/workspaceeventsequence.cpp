// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspaceeventsequence.h"

#include <dfm-framework/dpf.h>

#include <QPainter>
#include <QRectF>
#include <QAbstractItemView>
#include <QList>

Q_DECLARE_METATYPE(QRectF *)
Q_DECLARE_METATYPE(QPainter *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QAbstractItemView::SelectionMode> *)
Q_DECLARE_METATYPE(dfmbase::ElideTextLayout *)

DPF_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPWORKSPACE_NAMESPACE) };

WorkspaceEventSequence *WorkspaceEventSequence::instance()
{
    static WorkspaceEventSequence ins;
    return &ins;
}

bool WorkspaceEventSequence::doPaintListItem(int role, const FileInfoPointer &info, QPainter *painter, QRectF *rect)
{
    return dpfHookSequence->run(kCurrentEventSpace, "hook_Delegate_PaintListItem", role, info, painter, rect);
}

bool WorkspaceEventSequence::doIconItemLayoutText(const FileInfoPointer &info, dfmbase::ElideTextLayout *layout)
{
    return dpfHookSequence->run(kCurrentEventSpace, "hook_Delegate_LayoutText", info, layout);
}

bool WorkspaceEventSequence::doCheckDragTarget(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    return dpfHookSequence->run(kCurrentEventSpace, "hook_DragDrop_CheckDragDropAction", urls, urlTo, action);
}

bool WorkspaceEventSequence::doFetchSelectionModes(const QUrl &url, QList<QAbstractItemView::SelectionMode> *modes)
{
    return dpfHookSequence->run(kCurrentEventSpace, "hook_View_FetchSupportSelectionModes", url, modes);
}

bool WorkspaceEventSequence::doFetchCustomColumnRoles(const QUrl &rootUrl, QList<ItemRoles> *roleList)
{
    return dpfHookSequence->run(kCurrentEventSpace, "hook_Model_FetchCustomColumnRoles", rootUrl, roleList);
}

bool WorkspaceEventSequence::doFetchCustomRoleDiaplayName(const QUrl &rootUrl, const ItemRoles role, QString *displayName)
{
    return dpfHookSequence->run(kCurrentEventSpace, "hook_Model_FetchCustomRoleDisplayName", rootUrl, role, displayName);
}

bool WorkspaceEventSequence::doCheckTransparent(const QUrl &url, TransparentStatus *status)
{
    return dpfHookSequence->run(kCurrentEventSpace, "hook_Delegate_CheckTransparent", url, status);
}

WorkspaceEventSequence::WorkspaceEventSequence(QObject *parent)
    : QObject(parent)
{
}
