// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEEVENTSEQUENCE_H
#define WORKSPACEEVENTSEQUENCE_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/elidetextlayout.h>
#include <dfm-framework/dpf.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QAbstractItemView>

namespace dfmplugin_workspace {

class WorkspaceEventSequence : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceEventSequence)

public:
    static WorkspaceEventSequence *instance();

    bool doPaintListItem(int role, const FileInfoPointer &info, QPainter *painter, QRectF *rect);
    bool doIconItemLayoutText(const FileInfoPointer &info, dfmbase::ElideTextLayout *layout);
    bool doCheckDragTarget(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool doFetchSelectionModes(const QUrl &url, QList<QAbstractItemView::SelectionMode> *modes);
    bool doFetchCustomColumnRoles(const QUrl &rootUrl, QList<DFMGLOBAL_NAMESPACE::ItemRoles> *roleList);
    bool doFetchCustomRoleDiaplayName(const QUrl &rootUrl, const DFMGLOBAL_NAMESPACE::ItemRoles role, QString *displayName);
    bool doCheckTransparent(const QUrl &url, DFMGLOBAL_NAMESPACE::TransparentStatus *status);

private:
    explicit WorkspaceEventSequence(QObject *parent = nullptr);
};

}

#endif   // WORKSPACEEVENTSEQUENCE_H
