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
#include "workspacehelper.h"
#include "views/fileview.h"
#include "views/workspacewidget.h"
#include "models/filesortfilterproxymodel.h"
#include "models/fileviewmodel.h"
#include "events/workspaceeventcaller.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/application/application.h"

#include <dfm-framework/dpf.h>

#include <QDir>
#include <QTimer>

using namespace dfmplugin_workspace;
DFMBASE_USE_NAMESPACE

QMap<quint64, WorkspaceWidget *> WorkspaceHelper::kWorkspaceMap {};
QMap<QString, FileViewRoutePrehaldler> WorkspaceHelper::kPrehandlers {};

QMap<quint64, QPair<QUrl, QUrl>> WorkspaceHelper::kSelectionAndRenameFile {};
QMap<quint64, QPair<QUrl, QUrl>> WorkspaceHelper::kSelectionFile {};

void WorkspaceHelper::registerTopWidgetCreator(const WorkspaceHelper::KeyType &scheme, const WorkspaceHelper::TopWidgetCreator &creator)
{
    if (isRegistedTopWidget(scheme))
        return;

    topWidgetCreators.insert(scheme, creator);
}

bool WorkspaceHelper::isRegistedTopWidget(const WorkspaceHelper::KeyType &scheme) const
{
    return topWidgetCreators.contains(scheme);
}

CustomTopWidgetInterface *WorkspaceHelper::createTopWidgetByUrl(const QUrl &url)
{
    const KeyType &theType = url.scheme();
    if (!topWidgetCreators.contains(theType)) {
        qWarning() << "Scheme: " << theType << "not registered!";
        return nullptr;
    }
    return topWidgetCreators.value(theType)();
}

CustomTopWidgetInterface *WorkspaceHelper::createTopWidgetByScheme(const QString &scheme)
{
    if (!topWidgetCreators.contains(scheme)) {
        qWarning() << "Scheme: " << scheme << "not registered!";
        return nullptr;
    }
    return topWidgetCreators.value(scheme)();
}

void WorkspaceHelper::setCustomTopWidgetVisible(quint64 windowId, const QString &scheme, bool visible)
{
    WorkspaceWidget *workspaceWidget = findWorkspaceByWindowId(windowId);
    if (workspaceWidget) {
        workspaceWidget->setCustomTopWidgetVisible(scheme, visible);
    }
}

void WorkspaceHelper::setFilterData(quint64 windowId, const QUrl &url, const QVariant &data)
{
    emit requestSetViewFilterData(windowId, url, data);
}

void WorkspaceHelper::setFilterCallback(quint64 windowId, const QUrl &url, const FileViewFilterCallback callback)
{
    emit requestSetViewFilterCallback(windowId, url, callback);
}

void WorkspaceHelper::setWorkspaceMenuScene(const QString &scheme, const QString &scene)
{
    if (!scheme.isEmpty() && !scene.isEmpty())
        menuSceneMap[scheme] = scene;
}

void WorkspaceHelper::setDefaultViewMode(const QString &scheme, const Global::ViewMode mode)
{
    if (!scheme.isEmpty())
        defaultViewMode[scheme] = mode;
}

void WorkspaceHelper::setSelectionMode(const quint64 windowID, const QAbstractItemView::SelectionMode &mode)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setSelectionMode(mode);
}

void WorkspaceHelper::setEnabledSelectionModes(const quint64 windowID, const QList<QAbstractItemView::SelectionMode> &modes)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setEnabledSelectionModes(modes);
}

void WorkspaceHelper::setViewDragEnabled(const quint64 windowID, const bool enable)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setDragEnabled(enable);
}

void WorkspaceHelper::setViewDragDropMode(const quint64 windowID, const QAbstractItemView::DragDropMode mode)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setDragDropMode(mode);
}

WorkspaceHelper *WorkspaceHelper::instance()
{
    static WorkspaceHelper helper;
    return &helper;
}

WorkspaceWidget *WorkspaceHelper::findWorkspaceByWindowId(quint64 windowId)
{
    if (!kWorkspaceMap.contains(windowId))
        return nullptr;

    return kWorkspaceMap[windowId];
}

void WorkspaceHelper::closeTab(const QUrl &url)
{
    for (auto iter = kWorkspaceMap.cbegin(); iter != kWorkspaceMap.cend(); ++iter) {
        if (iter.value())
            iter.value()->closeTab(iter.key(), url);
    }
}

void WorkspaceHelper::addWorkspace(quint64 windowId, WorkspaceWidget *workspace)
{
    QMutexLocker locker(&WorkspaceHelper::mutex());
    if (!kWorkspaceMap.contains(windowId))
        kWorkspaceMap.insert(windowId, workspace);
}

void WorkspaceHelper::removeWorkspace(quint64 windowId)
{
    QMutexLocker locker(&WorkspaceHelper::mutex());
    if (kWorkspaceMap.contains(windowId))
        kWorkspaceMap.remove(windowId);
}

quint64 WorkspaceHelper::windowId(const QWidget *sender)
{
    return FMWindowsIns.findWindowId(sender);
}

void WorkspaceHelper::switchViewMode(quint64 windowId, int viewMode)
{
    emit viewModeChanged(windowId, viewMode);
}

void WorkspaceHelper::addScheme(const QString &scheme)
{
    ViewFactory::regClass<FileView>(scheme);
}

void WorkspaceHelper::openUrlInNewTab(quint64 windowId, const QUrl &url)
{
    emit openNewTab(windowId, url);
}

void WorkspaceHelper::actionNewWindow(const QList<QUrl> &urls)
{
    WorkspaceEventCaller::sendOpenWindow(urls);
}

void WorkspaceHelper::actionNewTab(quint64 windowId, const QUrl &url)
{
    openUrlInNewTab(windowId, url);
}

QString WorkspaceHelper::findMenuScene(const QString &scheme)
{
    if (menuSceneMap.contains(scheme))
        return menuSceneMap[scheme];

    return QString();
}

Global::ViewMode WorkspaceHelper::findViewMode(const QString &scheme)
{
    if (defaultViewMode.contains(scheme))
        return defaultViewMode[scheme];

    return static_cast<Global::ViewMode>(Application::instance()->appAttribute(Application::kViewMode).toInt());
}

void WorkspaceHelper::selectFiles(quint64 windowId, const QList<QUrl> &files)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        view->selectFiles(files);
}

void WorkspaceHelper::selectAll(quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        view->selectAll();
}

void WorkspaceHelper::setSort(quint64 windowId, Global::ItemRoles role)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view) {
        Qt::SortOrder order = view->model()->sortOrder();
        view->setSort(role, order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder);
    }
}

Global::ItemRoles WorkspaceHelper::sortRole(quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        return static_cast<Global::ItemRoles>(view->model()->sortRole());

    return Global::ItemRoles::kItemUnknowRole;
}

bool WorkspaceHelper::reigsterViewRoutePrehandler(const QString &scheme, const FileViewRoutePrehaldler prehandler)
{
    if (kPrehandlers.contains(scheme))
        return false;
    kPrehandlers.insert(scheme, prehandler);
    return true;
}

bool WorkspaceHelper::haveViewRoutePrehandler(const QString &scheme) const
{
    return kPrehandlers.contains(scheme);
}

FileViewRoutePrehaldler WorkspaceHelper::viewRoutePrehandler(const QString &scheme)
{
    return kPrehandlers.value(scheme, nullptr);
}

void WorkspaceHelper::closePersistentEditor(const quint64 windowID)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view && view->state() == 3) {   // protected enum, 3 means EditingState.
        view->closePersistentEditor(view->currentIndex());
    }
}

void WorkspaceHelper::setViewFilter(const quint64 windowID, const QDir::Filters filter)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setFilters(filter);
}

void WorkspaceHelper::setNameFilter(const quint64 windowID, const QStringList &filter)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setNameFilters(filter);
}

void WorkspaceHelper::setReadOnly(const quint64 windowID, const bool readOnly)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setReadOnly(readOnly);
}

int WorkspaceHelper::getViewFilter(const quint64 windowID)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        return view->getFilters();

    return QDir::NoFilter;
}

QStringList WorkspaceHelper::getNameFilter(const quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        return view->model()->getNameFilters();

    return {};
}

void WorkspaceHelper::laterRequestSelectFiles(const QList<QUrl> &urls)
{
    QTimer::singleShot(qMax(urls.count() * (10 + urls.count() / 150), 200), this, [=] {
        emit requestSelectFiles(urls);
    });
}

void WorkspaceHelper::fileUpdate(const QUrl &url)
{
    emit requestFileUpdate(url);
}

void WorkspaceHelper::installWorkspaceWidgetToWindow(const quint64 windowID)
{
    WorkspaceWidget *widget = nullptr;
    {
        QMutexLocker locker(&WorkspaceHelper::mutex());
        widget = kWorkspaceMap.value(windowID);
    }

    auto window = FMWindowsIns.findWindowById(windowID);
    if (!window || !widget)
        return;

    window->installWorkSpace(widget);

    connect(window, &FileManagerWindow::reqActivateNextTab, widget, &WorkspaceWidget::onNextTab);
    connect(window, &FileManagerWindow::reqActivatePreviousTab, widget, &WorkspaceWidget::onPreviousTab);
    connect(window, &FileManagerWindow::reqCloseCurrentTab, widget, &WorkspaceWidget::onCloseCurrentTab);
    connect(window, &FileManagerWindow::reqActivateTabByIndex, widget, &WorkspaceWidget::onSetCurrentTabIndex);
    connect(window, &FileManagerWindow::reqRefresh, widget, &WorkspaceWidget::onRefreshCurrentView);
}

WorkspaceHelper::WorkspaceHelper(QObject *parent)
    : QObject(parent)
{
}

QMutex &WorkspaceHelper::mutex()
{
    static QMutex m;
    return m;
}

FileView *WorkspaceHelper::findFileViewByWindowID(const quint64 windowID)
{
    WorkspaceWidget *workspaceWidget = findWorkspaceByWindowId(windowID);
    if (workspaceWidget) {
        FileView *view = dynamic_cast<FileView *>(workspaceWidget->currentView());
        return view;
    }
    return {};
}
