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
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "workspace/workspace_defines.h"
#include "dfm_global_defines.h"

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
    void setFilterCallback(quint64 windowId, const QUrl &url, const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback);
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
    bool schemeViewIsFileView(const QString &scheme);
    void openUrlInNewTab(quint64 windowId, const QUrl &viewMode);
    void actionNewWindow(const QList<QUrl> &urls);
    void actionNewTab(quint64 windowId, const QUrl &url);
    QString findMenuScene(const QString &scheme);
    DFMBASE_NAMESPACE::Global::ViewMode findViewMode(const QString &scheme);
    void selectFiles(quint64 windowId, const QList<QUrl> &files);
    void selectAll(quint64 windowId);
    void setSort(quint64 windowId, DFMBASE_NAMESPACE::Global::ItemRoles role);
    DFMBASE_NAMESPACE::Global::ItemRoles sortRole(quint64 windowId);

    bool reigsterViewRoutePrehandler(const QString &scheme, const DSB_FM_NAMESPACE::Workspace::FileViewRoutePrehaldler prehandler);
    DSB_FM_NAMESPACE::Workspace::FileViewRoutePrehaldler viewRoutePrehandler(const QString &scheme);

    void closePersistentEditor(const quint64 windowID, const QModelIndex &index);

    void setViewFilter(const quint64 windowID, const QDir::Filters filter);
    void setNameFilter(const quint64 windowID, const QStringList &filter);
    void setReadOnly(const quint64 windowID, const bool readOnly);

    int getViewFilter(const quint64 windowID);

    void laterRequestSelectFiles(const QList<QUrl> &urls);

    void fileUpdate(const QUrl &url);

    static QMap<quint64, QPair<QUrl, QUrl>> kSelectionAndRenameFile;   //###: for creating new file.
    static QMap<quint64, QPair<QUrl, QUrl>> kSelectionFile;   //###: rename a file which must be existance.

signals:
    void viewModeChanged(quint64 windowId, int viewMode);
    void openNewTab(quint64 windowId, const QUrl &url);
    void requestSetViewFilterData(quint64 windowId, const QUrl &url, const QVariant &data);
    void requestSetViewFilterCallback(quint64 windowId, const QUrl &url, const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback);
    void requestSelectFiles(const QList<QUrl> &urlList);
    void trashStateChanged();
    void requestFileUpdate(const QUrl &url);

private:
    explicit WorkspaceHelper(QObject *parent = nullptr);
    static QMutex &mutex();
    static QMap<quint64, WorkspaceWidget *> kWorkspaceMap;
    static QMap<QString, DSB_FM_NAMESPACE::Workspace::FileViewRoutePrehaldler> kPrehandlers;

    FileView *findFileViewByWindowID(const quint64 windowID);

private:
    TopWidgetCreatorMap topWidgetCreators;
    MenuSceneMap menuSceneMap;
    DefaultViewMode defaultViewMode;

    Q_DISABLE_COPY(WorkspaceHelper)
};

}

#endif   // DETAILSPACEHELPER_H
