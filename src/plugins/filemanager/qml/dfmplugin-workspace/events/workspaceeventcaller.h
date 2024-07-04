// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEEVENTCALLER_H
#define WORKSPACEEVENTCALLER_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/base/schemefactory.h>

#include <QObject>
#include <QPainter>
#include <QItemSelection>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_workspace {

class WorkspaceEventCaller
{
    WorkspaceEventCaller() = delete;

public:
    static void sendOpenWindow(const QList<QUrl> &urls, const bool isNew = true);
    static void sendChangeCurrentUrl(const quint64 windowId, const QUrl &url);
    static void sendOpenAsAdmin(const QUrl &url);

    static void sendTabAdded(const quint64 windowID);
    static void sendTabChanged(const quint64 windowID, const int index);
    static void sendTabMoved(const quint64 windowID, const int from, const int to);
    static void sendTabRemoved(const quint64 windowID, const int index);
    static void sendShowCustomTopWidget(const quint64 windowId, const QString &scheme, bool visible);

    static void sendPaintEmblems(QPainter *painter, const QRectF &paintArea, const FileInfoPointer &info);

    static void sendViewSelectionChanged(const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected);

    static bool sendRenameStartEdit(const quint64 &winId, const QUrl &url);

    static bool sendRenameEndEdit(const quint64 &winId, const QUrl &url);
    static bool sendViewItemClicked(const QVariantMap &data);
    static void sendEnterDirReportLog(const QVariantMap &data);
    static void sendModelFilesEmpty();
};

}

Q_DECLARE_METATYPE(QPainter *)

#endif   // WORKSPACEEVENTCALLER_H
