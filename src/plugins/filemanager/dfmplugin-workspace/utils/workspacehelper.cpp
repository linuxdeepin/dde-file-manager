// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacehelper.h"
#include "views/fileview.h"
#include "views/workspacewidget.h"
#include "models/fileviewmodel.h"
#include "events/workspaceeventcaller.h"
#include "utils/filedatamanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <QDir>
#include <QTimer>

using namespace dfmplugin_workspace;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

QMap<quint64, WorkspaceWidget *> WorkspaceHelper::kWorkspaceMap {};
QMap<QString, FileViewRoutePrehaldler> WorkspaceHelper::kPrehandlers {};

QMap<quint64, QPair<QUrl, QUrl>> WorkspaceHelper::kSelectionAndRenameFile {};
QMap<quint64, QPair<QUrl, QUrl>> WorkspaceHelper::kSelectionFile {};

void WorkspaceHelper::registerTopWidgetCreator(const WorkspaceHelper::KeyType &scheme, const WorkspaceHelper::TopWidgetCreator &creator)
{
    if (isRegistedTopWidget(scheme)) {
        fmDebug() << "Top widget creator already registered for scheme:" << scheme;
        return;
    }

    topWidgetCreators.insert(scheme, creator);
    fmInfo() << "Top widget creator registered for scheme:" << scheme;
}

bool WorkspaceHelper::isRegistedTopWidget(const WorkspaceHelper::KeyType &scheme) const
{
    return topWidgetCreators.contains(scheme);
}

CustomTopWidgetInterface *WorkspaceHelper::createTopWidgetByUrl(const QUrl &url)
{
    const KeyType &theType = url.scheme();
    if (!topWidgetCreators.contains(theType)) {
        fmWarning() << "Scheme: " << theType << "not registered!";
        return nullptr;
    }

    fmDebug() << "Creating top widget for URL:" << url.toString();
    return topWidgetCreators.value(theType)();
}

CustomTopWidgetInterface *WorkspaceHelper::createTopWidgetByScheme(const QString &scheme)
{
    if (!topWidgetCreators.contains(scheme)) {
        fmWarning() << "Scheme: " << scheme << "not registered!";
        return nullptr;
    }

    fmDebug() << "Creating top widget for scheme:" << scheme;
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
    fmDebug() << "WorkspaceHelper: setting filter data for window" << windowId << "URL" << url.toString();
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        view->setFilterData(url, data);
}

void WorkspaceHelper::setFilterCallback(quint64 windowId, const QUrl &url, const FileViewFilterCallback callback)
{
    fmDebug() << "WorkspaceHelper: setting filter callback for window" << windowId << "URL" << url.toString();
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        view->setFilterCallback(url, callback);
}

void WorkspaceHelper::setWorkspaceMenuScene(const QString &scheme, const QString &scene)
{
    if (!scheme.isEmpty() && !scene.isEmpty())
        menuSceneMap[scheme] = scene;
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

void WorkspaceHelper::registerCustomViewProperty(const QString &scheme, const QVariantMap &propertise)
{
    if (scheme.isEmpty()) {
        fmWarning() << "Cannot register custom view property with empty scheme";
        return;
    }

    if (customViewPropertyMap.contains(scheme)) {
        customViewPropertyMap[scheme] = CustomViewProperty(propertise);
        fmDebug() << "Updated custom view property for scheme:" << scheme;
    } else {
        customViewPropertyMap.insert(scheme, CustomViewProperty(propertise));
        fmInfo() << "Registered custom view property for scheme:" << scheme;
    }
}

CustomViewProperty WorkspaceHelper::findCustomViewProperty(const QString &scheme) const
{
    if (customViewPropertyMap.contains(scheme))
        return customViewPropertyMap[scheme];

    return CustomViewProperty();
}

bool WorkspaceHelper::isViewModeSupported(const QString &scheme, const dfmbase::Global::ViewMode mode) const
{
    auto customProperty = findCustomViewProperty(scheme);
    switch (mode) {
    case Global::ViewMode::kTreeMode:
        return customProperty.supportTreeMode;
    case Global::ViewMode::kListMode:
        return customProperty.supportListMode;
    case Global::ViewMode::kIconMode:
        return customProperty.supportIconMode;
    default:
        return true;
    }
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
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    WorkspaceEventCaller::sendCloseTab(url);
}

void WorkspaceHelper::addWorkspace(quint64 windowId, WorkspaceWidget *workspace)
{
    QMutexLocker locker(&WorkspaceHelper::mutex());
    if (!kWorkspaceMap.contains(windowId)) {
        kWorkspaceMap.insert(windowId, workspace);
        fmDebug() << "Workspace added for window ID:" << windowId;
    } else {
        fmDebug() << "Workspace already exists for window ID:" << windowId;
    }
}

void WorkspaceHelper::removeWorkspace(quint64 windowId)
{
    QMutexLocker locker(&WorkspaceHelper::mutex());
    if (kWorkspaceMap.contains(windowId)) {
        kWorkspaceMap.remove(windowId);
        fmDebug() << "Workspace removed for window ID:" << windowId;
    } else {
        fmDebug() << "No workspace found for window ID:" << windowId;
    }
}

quint64 WorkspaceHelper::windowId(const QWidget *sender)
{
    return FMWindowsIns.findWindowId(sender);
}

void WorkspaceHelper::switchViewMode(quint64 windowId, int viewMode)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view) {
        fmDebug() << "Switching view mode to" << viewMode << "for window ID:" << windowId;
        view->viewModeChanged(windowId, viewMode);
    } else {
        fmWarning() << "No file view found for window ID:" << windowId;
    }
}

void WorkspaceHelper::addScheme(const QString &scheme)
{
    fmInfo() << "Adding scheme:" << scheme;
    ViewFactory::regClass<FileView>(scheme);
}

void WorkspaceHelper::actionNewWindow(const QList<QUrl> &urls)
{
    WorkspaceEventCaller::sendOpenWindow(urls);
}

QString WorkspaceHelper::findMenuScene(const QString &scheme)
{
    if (menuSceneMap.contains(scheme))
        return menuSceneMap[scheme];

    return QString();
}

Global::ViewMode WorkspaceHelper::findViewMode(const QString &scheme)
{
    auto customProperty = findCustomViewProperty(scheme);
    if (customProperty.defaultViewMode != ViewMode::kNoneMode)
        return customProperty.defaultViewMode;

    ViewMode mode = static_cast<ViewMode>(Application::instance()->appAttribute(Application::kViewMode).toInt());

    if (mode != ViewMode::kIconMode && mode != ViewMode::kListMode && mode != ViewMode::kTreeMode
        && mode != ViewMode::kExtendMode && mode != ViewMode::kAllViewMode) {
        fmWarning() << "Config view mode is invalid, reset it to icon mode.";
        mode = Global::ViewMode::kIconMode;
        Application::instance()->setAppAttribute(Application::kViewMode, static_cast<int>(mode));
    }

    return mode;
}

void WorkspaceHelper::selectFiles(quint64 windowId, const QList<QUrl> &files)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view) {
        fmDebug() << "Selecting" << files.size() << "files for window ID:" << windowId;
        view->selectFiles(files);
    } else {
        fmWarning() << "No file view found for window ID:" << windowId;
    }
}

void WorkspaceHelper::selectAll(quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view) {
        fmDebug() << "Selecting all files for window ID:" << windowId;
        view->selectAll();
    } else {
        fmWarning() << "No file view found for window ID:" << windowId;
    }
}

void WorkspaceHelper::reverseSelect(quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        view->reverseSelect();
}

void WorkspaceHelper::setSort(quint64 windowId, Global::ItemRoles role)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view) {
        Qt::SortOrder order = view->model()->sortOrder();
        auto oldRole = view->model()->sortRole();
        order = oldRole != role               ? Qt::AscendingOrder
                : order == Qt::AscendingOrder ? Qt::DescendingOrder
                                              : Qt::AscendingOrder;

        fmDebug() << "Sorting by role:" << role << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending")
                  << "old role:" << oldRole;

        view->setSort(role, order);
    }
}

Global::ItemRoles WorkspaceHelper::sortRole(quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        return static_cast<Global::ItemRoles>(view->model()->sortRole());

    return Global::ItemRoles::kItemUnknowRole;
}

// Modern grouping interface
void WorkspaceHelper::setGroupingStrategy(quint64 windowId, const QString &strategyName)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view) {
        fmInfo() << "WorkspaceHelper: Setting grouping strategy" << strategyName << "for window" << windowId;
        QString oldStrategy = view->model()->groupingStrategy();
        Qt::SortOrder order = view->model()->groupingOrder();
        fmInfo() << "Setting grouping strategy:" << strategyName << "old strategy:" << oldStrategy;

        // Always set the strategy first (including "NoGroupStrategy")
        if (oldStrategy == strategyName) {
            // User clicked the same strategy - toggle sort order (unless it's NoGroupStrategy)
            if (strategyName != GroupStrategty::kNoGroup) {
                order = (order == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
                fmInfo() << "Toggling grouping order for strategy:" << strategyName
                         << "to" << (order == Qt::AscendingOrder ? "Ascending" : "Descending");
            }
            // For NoGroupStrategy, clicking again just reconfirms the disable state
        } else {
            // Different strategy - set default ascending order for new strategy
            order = Qt::AscendingOrder;
        }

        view->setGroup(strategyName, order);
        fmInfo() << "Grouping set with strategy:" << strategyName
                 << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending");
    }
}

QString WorkspaceHelper::getGroupingStrategy(quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view) {
        return view->model()->groupingStrategy();
    }

    return {};
}

QList<ItemRoles> WorkspaceHelper::columnRoles(quint64 windowId)
{
    FileView *view = findFileViewByWindowID(windowId);
    if (view)
        return view->model()->getColumnRoles();

    return {};
}

bool WorkspaceHelper::reigsterViewRoutePrehandler(const QString &scheme, const FileViewRoutePrehaldler prehandler)
{
    if (kPrehandlers.contains(scheme)) {
        fmDebug() << "View route prehandler already registered for scheme:" << scheme;
        return false;
    }
    kPrehandlers.insert(scheme, prehandler);
    fmInfo() << "View route prehandler registered for scheme:" << scheme;
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
    if (view) {
        QDir::Filters filters = filter;
        if (Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool()) {
            filters = filters | QDir::Hidden;
        } else {
            filters = filters & QDir::Filter(~QDir::Hidden);
        }
        view->model()->setFilters(filters);
    }
}

void WorkspaceHelper::setNameFilter(const quint64 windowID, const QStringList &filter)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->model()->setNameFilters(filter);
}

void WorkspaceHelper::setReadOnly(const quint64 windowID, const bool readOnly)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->model()->setReadOnly(readOnly);
}

int WorkspaceHelper::getViewFilter(const quint64 windowID)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        return view->model()->getFilters();

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
    QTimer::singleShot(qMin(800 + (urls.count() / 100) * 50, qMax(urls.count() * (10 + urls.count() / 150), 500)), this, [=] {
        emit requestSelectFiles(urls);
    });
}

void WorkspaceHelper::fileUpdate(const QUrl &url)
{
    fmDebug() << "WorkspaceHelper: updating file for URL" << url.toString();
    for (const auto &wind : kWorkspaceMap) {
        if (wind) {
            FileView *view = dynamic_cast<FileView *>(wind->currentView());
            if (view) {
                view->model()->updateFile(url);
            }
        }
    }
}

void WorkspaceHelper::updateRootFile(const QList<QUrl> urls)
{
    // TODO(liuyangming): rename this func. clean root data cache in FileDataManager about urls.
    //    QMutexLocker locker(&WorkspaceHelper::mutex());
    //    for (const auto &wind : kWorkspaceMap) {
    //        if (wind) {
    //            FileView *view = dynamic_cast<FileView *>(wind->currentView());
    //            if (view)
    //                view->updateRootInfo(urls);
    //        }
    //    }
}

void WorkspaceHelper::registerFileView(const QString &scheme)
{
    fmInfo() << "Registering file view for scheme:" << scheme;
    ViewFactory::regClass<FileView>(scheme);

    if (!registeredFileViewScheme.contains(scheme)) {
        registeredFileViewScheme.append(scheme);
        fmDebug() << "File view scheme added to registry:" << scheme;
    }
}

bool WorkspaceHelper::registeredFileView(const QString &scheme) const
{
    return registeredFileViewScheme.contains(scheme);
}

void WorkspaceHelper::setUndoFiles(const QList<QUrl> &files)
{
    undoFiles = files;
}

QList<QUrl> WorkspaceHelper::filterUndoFiles(const QList<QUrl> &urlList) const
{
    QList<QUrl> urls(urlList);
    for (auto url : urlList) {
        for (auto undoFile : undoFiles) {
            if (UniversalUtils::urlEquals(url, undoFile)) {
                urls.removeAll(url);
                break;
            }
        }
    }

    return urls;
}

void WorkspaceHelper::setAlwaysOpenInCurrentWindow(const quint64 windowID)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->setAlwaysOpenInCurrentWindow(true);
}

void WorkspaceHelper::aboutToChangeViewWidth(const quint64 windowID, int deltaWidth)
{
    FileView *view = findFileViewByWindowID(windowID);
    if (view)
        view->aboutToChangeWidth(deltaWidth);
}

void WorkspaceHelper::registerFocusFileViewDisabled(const QString &scheme)
{
    focusFileViewDisabledScheme.append(scheme);
}

bool WorkspaceHelper::isFocusFileViewDisabled(const QString &scheme) const
{
    return focusFileViewDisabledScheme.contains(scheme);
}

void WorkspaceHelper::registerLoadStrategy(const QString &scheme, DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy strategy)
{
    loadStrategyMap[scheme] = strategy;
    fmDebug() << "Load strategy registered for scheme:" << scheme << "strategy:" << static_cast<int>(strategy);
}

DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy WorkspaceHelper::getLoadStrategy(const QString &scheme)
{
    return loadStrategyMap.value(scheme, DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy::kCreateNew);
}

void WorkspaceHelper::installWorkspaceWidgetToWindow(const quint64 windowID)
{
    WorkspaceWidget *widget = nullptr;
    {
        QMutexLocker locker(&WorkspaceHelper::mutex());
        widget = kWorkspaceMap.value(windowID);
    }

    auto window = FMWindowsIns.findWindowById(windowID);
    if (!window || !widget) {
        fmWarning() << "Cannot install workspace widget - window or widget not found for ID:" << windowID;
        return;
    }

    fmInfo() << "Installing workspace widget to window ID:" << windowID;
    window->installWorkSpace(widget);

    connect(window, &FileManagerWindow::reqCreateWindow, widget, &WorkspaceWidget::onCreateNewWindow);
    connect(window, &FileManagerWindow::reqRefresh, widget, &WorkspaceWidget::onRefreshCurrentView);
    connect(window, &FileManagerWindow::currentViewStateChanged, widget, &WorkspaceWidget::handleViewStateChanged);
    connect(window, &FileManagerWindow::aboutToPlaySplitterAnimation, widget, &WorkspaceWidget::handleAboutToPlaySplitterAnim);
}

void WorkspaceHelper::handleRefreshDir(const QList<QUrl> &urls)
{
    for (auto url : urls) {
        for (auto workspace : kWorkspaceMap) {
            if (UniversalUtils::urlEquals(url, workspace->currentUrl())) {
                workspace->onRefreshCurrentView();
            }
        }
    }
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
        if (!view) {
            fmDebug() << "Current view is not a FileView for window ID:" << windowID;
        }
        return view;
    }
    fmDebug() << "No workspace widget found for window ID:" << windowID;
    return {};
}

QUrl WorkspaceHelper::transformViewModeUrl(const QUrl &url) const
{
    auto callback = findCustomViewProperty(url.scheme()).viewModelUrlCallback;
    return callback ? callback(url) : url;
}

QVariant WorkspaceHelper::getFileViewStateValue(const QUrl &url, const QString &key, const QVariant &defaultValue) const
{
    QUrl viewModeUrl = transformViewModeUrl(url);
    QMap<QString, QVariant> valueMap = Application::appObtuselySetting()->value("FileViewState", viewModeUrl).toMap();
    return valueMap.value(key, defaultValue);
}

void WorkspaceHelper::setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value)
{
    QUrl viewModeUrl = transformViewModeUrl(url);
    QVariantMap map = Application::appObtuselySetting()->value("FileViewState", viewModeUrl).toMap();
    map[key] = value;
    Application::appObtuselySetting()->setValue("FileViewState", viewModeUrl, map);
}
