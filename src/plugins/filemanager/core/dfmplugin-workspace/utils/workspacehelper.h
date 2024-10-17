// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACEHELPER_H
#define DETAILSPACEHELPER_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/dfm_global_defines.h>

#include <QDir>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QAbstractItemView>

#include <functional>

QT_BEGIN_NAMESPACE
class QFrame;
QT_END_NAMESPACE
namespace dfmplugin_workspace {

class CustomTopWidgetInterface;
class WorkspaceWidget;
class FileView;
class WorkspaceHelper : public QObject
{
    Q_OBJECT
public:
    static WorkspaceHelper *instance();

    using KeyType = QString;
    using TopWidgetCreator = std::function<CustomTopWidgetInterface *()>;
    using TopWidgetCreatorMap = QMap<KeyType, TopWidgetCreator>;
    using MenuSceneMap = QMap<QString, QString>;
    using DefaultViewMode = QMap<QString, DFMBASE_NAMESPACE::Global::ViewMode>;

    void registerTopWidgetCreator(const KeyType &scheme, const TopWidgetCreator &creator);
    bool isRegistedTopWidget(const KeyType &scheme) const;
    CustomTopWidgetInterface *createTopWidgetByUrl(const QUrl &url);
    CustomTopWidgetInterface *createTopWidgetByScheme(const QString &scheme);
    void setCustomTopWidgetVisible(quint64 windowId, const QString &scheme, bool visible);
    void setFilterData(quint64 windowId, const QUrl &url, const QVariant &data);
    void setFilterCallback(quint64 windowId, const QUrl &url, const FileViewFilterCallback callback);
    void setWorkspaceMenuScene(const QString &scheme, const QString &scene);
    void setDefaultViewMode(const QString &scheme, const DFMBASE_NAMESPACE::Global::ViewMode mode);
    void setSelectionMode(const quint64 windowID, const QAbstractItemView::SelectionMode &mode);
    void setEnabledSelectionModes(const quint64 windowID, const QList<QAbstractItemView::SelectionMode> &modes);
    void setViewDragEnabled(const quint64 windowID, const bool enable);
    void setViewDragDropMode(const quint64 windowID, const QAbstractItemView::DragDropMode mode);

    WorkspaceWidget *findWorkspaceByWindowId(quint64 windowId);
    void closeTab(const QUrl &url);
    void addWorkspace(quint64 windowId, WorkspaceWidget *workspace);
    void removeWorkspace(quint64 windowId);
    quint64 windowId(const QWidget *sender);
    void switchViewMode(quint64 windowId, int viewMode);
    void addScheme(const QString &scheme);
    void actionNewWindow(const QList<QUrl> &urls);
    QString findMenuScene(const QString &scheme);
    DFMBASE_NAMESPACE::Global::ViewMode findViewMode(const QString &scheme);
    void selectFiles(quint64 windowId, const QList<QUrl> &files);
    void selectAll(quint64 windowId);
    void reverseSelect(quint64 windowId);
    void setSort(quint64 windowId, DFMBASE_NAMESPACE::Global::ItemRoles role);
    DFMBASE_NAMESPACE::Global::ItemRoles sortRole(quint64 windowId);
    QList<DFMGLOBAL_NAMESPACE::ItemRoles> columnRoles(quint64 windowId);

    bool reigsterViewRoutePrehandler(const QString &scheme, const FileViewRoutePrehaldler prehandler);
    bool haveViewRoutePrehandler(const QString &scheme) const;
    FileViewRoutePrehaldler viewRoutePrehandler(const QString &scheme);

    void closePersistentEditor(const quint64 windowID);

    void setViewFilter(const quint64 windowID, const QDir::Filters filter);
    void setNameFilter(const quint64 windowID, const QStringList &filter);
    void setReadOnly(const quint64 windowID, const bool readOnly);

    int getViewFilter(const quint64 windowID);
    QStringList getNameFilter(const quint64 windowId);

    void laterRequestSelectFiles(const QList<QUrl> &urls);

    void fileUpdate(const QUrl &url);
    void updateRootFile(const QList<QUrl> urls);

    void registerFileView(const QString &scheme);
    bool registeredFileView(const QString &scheme) const;

    void setNotSupportTreeView(const QString &scheme);
    bool supportTreeView(const QString &scheme) const;

    void setUndoFiles(const QList<QUrl> &files);
    QList<QUrl> filterUndoFiles(const QList<QUrl> &urlList) const;

    void setAlwaysOpenInCurrentWindow(const quint64 windowID);

    void aboutToChangeViewWidth(const quint64 windowID, int deltaWidth);

    static QMap<quint64, QPair<QUrl, QUrl>> kSelectionAndRenameFile;   //###: for creating new file.
    static QMap<quint64, QPair<QUrl, QUrl>> kSelectionFile;   //###: rename a file which must be existance.

public Q_SLOTS:
    void installWorkspaceWidgetToWindow(const quint64 windowID);
    void handleRefreshDir(const QList<QUrl> &urls);

signals:
    void requestSelectFiles(const QList<QUrl> &urlList);
    void trashStateChanged();

private:
    explicit WorkspaceHelper(QObject *parent = nullptr);
    static QMutex &mutex();
    static QMap<quint64, WorkspaceWidget *> kWorkspaceMap;
    static QMap<QString, FileViewRoutePrehaldler> kPrehandlers;

    FileView *findFileViewByWindowID(const quint64 windowID);

private:
    TopWidgetCreatorMap topWidgetCreators;
    MenuSceneMap menuSceneMap;
    DefaultViewMode defaultViewMode;

    QList<QString> registeredFileViewScheme {};
    QList<QString> notSupportTreeView{};

    QList<QUrl> undoFiles {};

    Q_DISABLE_COPY(WorkspaceHelper)
};

}

#endif   // DETAILSPACEHELPER_H
