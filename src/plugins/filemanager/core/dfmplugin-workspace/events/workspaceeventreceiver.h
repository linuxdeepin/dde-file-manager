// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEEVENTRECEIVER_H
#define WORKSPACEEVENTRECEIVER_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QUrl>
#include <QDir>
#include <QAbstractItemView>

namespace dfmplugin_workspace {

class WorkspaceEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceEventReceiver)

public:
    ~WorkspaceEventReceiver();
    static WorkspaceEventReceiver *instance();

    void initConnection();

public slots:
    void handleTileBarSwitchModeTriggered(quint64 windowId, int mode);
    void handleOpenNewTabTriggered(quint64 windowId, const QUrl &url);
    void handleShowCustomTopWidget(quint64 windowId, const QString &scheme, bool visible);
    bool handleTabAddable(quint64 windowId);
    void handleCloseTabs(const QUrl &url);
    void handleSetTabAlias(const QUrl &url, const QString &name);
    void handleSelectFiles(quint64 windowId, const QList<QUrl> &files);
    void handleSelectAll(quint64 windowId);
    void handleReverseSelect(quint64 windowId);
    void handleSetSort(quint64 windowId, DFMBASE_NAMESPACE::Global::ItemRoles role);
    void handleNotSupportTreeView(const QString &scheme);

    void handleSetSelectionMode(const quint64 windowId, const QAbstractItemView::SelectionMode mode);
    void handleSetEnabledSelectionModes(const quint64 windowId, const QList<QAbstractItemView::SelectionMode> &modes);

    void handleSetViewDragEnabled(const quint64 windowId, const bool enabled);
    void handleSetViewDragDropMode(const quint64 windowId, const QAbstractItemView::DragDropMode mode);
    void handleClosePersistentEditor(const quint64 windowId);

    int handleGetViewFilter(const quint64 windowId);
    QStringList handleGetNameFilter(const quint64 windowId);
    void handleSetViewFilter(const quint64 windowId, const QDir::Filters &filters);
    void handleSetNameFilter(const quint64 windowId, const QStringList &filters);
    void handleSetReadOnly(const quint64 windowId, const bool readOnly);

    void handlePasteFileResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg);
    void handleMoveToTrashFileResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg);
    void handleRenameFileResult(const quint64 windowId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg);
    void handleFileUpdate(const QUrl &url);
    DFMBASE_NAMESPACE::Global::ItemRoles handleCurrentSortRole(quint64 windowId);
    QList<DFMGLOBAL_NAMESPACE::ItemRoles> handleColumnRoles(quint64 windowId);

    QRectF handleGetVisualGeometry(const quint64 windowID);
    QRectF handleGetViewItemRect(const quint64 windowID, const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role);
    void handleSetDefaultViewMode(const QString &scheme, const DFMBASE_NAMESPACE::Global::ViewMode mode);
    DFMBASE_NAMESPACE::Global::ViewMode handleGetDefaultViewMode(const QString &scheme);
    DFMBASE_NAMESPACE::Global::ViewMode handleGetCurrentViewMode(const quint64 windowID);

    void handleRegisterFileView(const QString &scheme);
    void handleRegisterMenuScene(const QString &scheme, const QString &scene);
    QString handleFindMenuScene(const QString &scheme);
    void handleRegisterCustomTopWidget(const QVariantMap &dataMap);
    bool handleGetCustomTopWidgetVisible(const quint64 windowID, const QString &scheme);
    bool handleCheckSchemeViewIsFileView(const QString &scheme);
    QList<QUrl> handleGetSelectedUrls(const quint64 windowID);

    void handleSetCustomFilterData(quint64 windowID, const QUrl &url, const QVariant &data);
    void handleSetCustomFilterCallback(quint64 windowID, const QUrl &url, const QVariant callback);
    bool handleRegisterRoutePrehandle(const QString &scheme, const FileViewRoutePrehaldler &prehandler);
    void handleRegisterDataCache(const QString &scheme);
    void handleSetAlwaysOpenInCurrentWindow(const quint64 windowID);

private:
    explicit WorkspaceEventReceiver(QObject *parent = nullptr);
};

}

#endif   // WORKSPACEEVENTRECEIVER_H
