// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspaceeventreceiver.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"
#include "views/workspacewidget.h"
#include "views/fileview.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>
#include <functional>

Q_DECLARE_METATYPE(QRectF *)

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPWORKSPACE_NAMESPACE) };

WorkspaceEventReceiver::WorkspaceEventReceiver(QObject *parent)
    : QObject(parent)
{
}

WorkspaceEventReceiver::~WorkspaceEventReceiver()
{
    // dpfSignalDispatcher->unsubscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged",
    //                                  WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged);
}

WorkspaceEventReceiver *WorkspaceEventReceiver::instance()
{
    static WorkspaceEventReceiver receiver;
    return &receiver;
}

void WorkspaceEventReceiver::initConnection()
{
    // signal event
    dpfSignalDispatcher->subscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged",
                                   WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged);

    // slot event
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_RegisterFileView",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterFileView);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_RegisterMenuScene",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterMenuScene);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_FindMenuScene",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleFindMenuScene);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_RegisterCustomTopWidget",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterCustomTopWidget);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_GetCustomTopWidgetVisible",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetCustomTopWidgetVisible);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_ShowCustomTopWidget",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleShowCustomTopWidget);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_CheckSchemeViewIsFileView",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleCheckSchemeViewIsFileView);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_RefreshDir",
                            WorkspaceHelper::instance(), &WorkspaceHelper::handleRefreshDir);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Tab_Addable",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTabAddable);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Tab_Close",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleCloseTabs);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Tab_SetAlias",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetTabAlias);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_NotSupportTreeView",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleNotSupportTreeView);

    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetVisualGeometry",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetVisualGeometry);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetViewItemRect",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetViewItemRect);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetCurrentViewMode",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetCurrentViewMode);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetDefaultViewMode",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetDefaultViewMode);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetDefaultViewMode",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetDefaultViewMode);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetSelectedUrls",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetSelectedUrls);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SelectFiles",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSelectFiles);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SelectAll",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSelectAll);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_ReverseSelect",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleReverseSelect);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetSelectionMode",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetSelectionMode);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetEnabledSelectionModes",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetEnabledSelectionModes);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetDragEnabled",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewDragEnabled);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetDragDropMode",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewDragDropMode);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetReadOnly",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetReadOnly);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetFilter",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetViewFilter);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetFilter",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetViewFilter);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_ClosePersistentEditor",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleClosePersistentEditor);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetAlwaysOpenInCurrentWindow",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetAlwaysOpenInCurrentWindow);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_SetCustomFilterData",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetCustomFilterData);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_SetCustomFilterCallback",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetCustomFilterCallback);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_RegisterRoutePrehandle",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterRoutePrehandle);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_FileUpdate",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleFileUpdate);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_SetNameFilter",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetNameFilter);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_GetNameFilter",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetNameFilter);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_CurrentSortRole",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleCurrentSortRole);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_ColumnRoles",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleColumnRoles);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_SetSort",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetSort);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_RegisterDataCache",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterDataCache);

    dpfSignalDispatcher->subscribe(GlobalEventType::kSwitchViewMode,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTileBarSwitchModeTriggered);
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewTab,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleOpenNewTabTriggered);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFileResult,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handlePasteFileResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyResult,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handlePasteFileResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRenameFileResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrashResult,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleMoveToTrashFileResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFilesResult,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleMoveToTrashFileResult);
}

void WorkspaceEventReceiver::handleTileBarSwitchModeTriggered(quint64 windowId, int mode)
{
    WorkspaceHelper::instance()->switchViewMode(windowId, mode);
}

void WorkspaceEventReceiver::handleOpenNewTabTriggered(quint64 windowId, const QUrl &url)
{
    WorkspaceHelper::instance()->openUrlInNewTab(windowId, url);
}

void WorkspaceEventReceiver::handleShowCustomTopWidget(quint64 windowId, const QString &scheme, bool visible)
{
    WorkspaceHelper::instance()->setCustomTopWidgetVisible(windowId, scheme, visible);
}

bool WorkspaceEventReceiver::handleTabAddable(quint64 windowId)
{
    auto widget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowId);
    if (!widget)
        return false;

    return widget->canAddNewTab();
}

void WorkspaceEventReceiver::handleCloseTabs(const QUrl &url)
{
    WorkspaceHelper::instance()->closeTab(url);
}

void WorkspaceEventReceiver::handleSetTabAlias(const QUrl &url, const QString &name)
{
    WorkspaceHelper::instance()->setTabAlias(url, name);
}

void WorkspaceEventReceiver::handleSelectFiles(quint64 windowId, const QList<QUrl> &files)
{
    WorkspaceHelper::instance()->selectFiles(windowId, files);
}

void WorkspaceEventReceiver::handleSelectAll(quint64 windowId)
{
    WorkspaceHelper::instance()->selectAll(windowId);
}

void WorkspaceEventReceiver::handleReverseSelect(quint64 windowId)
{
    WorkspaceHelper::instance()->reverseSelect(windowId);
}

void WorkspaceEventReceiver::handleSetSort(quint64 windowId, ItemRoles role)
{
    WorkspaceHelper::instance()->setSort(windowId, role);
}

void WorkspaceEventReceiver::handleNotSupportTreeView(const QString &scheme)
{
    WorkspaceHelper::instance()->setNotSupportTreeView(scheme);
}

void WorkspaceEventReceiver::handleSetSelectionMode(const quint64 windowId, const QAbstractItemView::SelectionMode mode)
{
    WorkspaceHelper::instance()->setSelectionMode(windowId, mode);
}

void WorkspaceEventReceiver::handleSetEnabledSelectionModes(const quint64 windowId, const QList<QAbstractItemView::SelectionMode> &modes)
{
    WorkspaceHelper::instance()->setEnabledSelectionModes(windowId, modes);
}

void WorkspaceEventReceiver::handleSetViewDragEnabled(const quint64 windowId, const bool enabled)
{
    WorkspaceHelper::instance()->setViewDragEnabled(windowId, enabled);
}

void WorkspaceEventReceiver::handleSetViewDragDropMode(const quint64 windowId, const QAbstractItemView::DragDropMode mode)
{
    WorkspaceHelper::instance()->setViewDragDropMode(windowId, mode);
}

void WorkspaceEventReceiver::handleClosePersistentEditor(const quint64 windowId)
{
    WorkspaceHelper::instance()->closePersistentEditor(windowId);
}

int WorkspaceEventReceiver::handleGetViewFilter(const quint64 windowId)
{
    return WorkspaceHelper::instance()->getViewFilter(windowId);
}

QStringList WorkspaceEventReceiver::handleGetNameFilter(const quint64 windowId)
{
    return WorkspaceHelper::instance()->getNameFilter(windowId);
}

void WorkspaceEventReceiver::handleSetViewFilter(const quint64 windowId, const QDir::Filters &filters)
{
    WorkspaceHelper::instance()->setViewFilter(windowId, filters);
}

void WorkspaceEventReceiver::handleSetNameFilter(const quint64 windowId, const QStringList &filters)
{
    WorkspaceHelper::instance()->setNameFilter(windowId, filters);
}

void WorkspaceEventReceiver::handleSetReadOnly(const quint64 windowId, const bool readOnly)
{
    WorkspaceHelper::instance()->setReadOnly(windowId, readOnly);
}

void WorkspaceEventReceiver::handlePasteFileResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(srcUrls)
    Q_UNUSED(ok)
    Q_UNUSED(errMsg)

    // if paste files from revocation operate, these files should not be selected.
    QList<QUrl> filterUrls = WorkspaceHelper::instance()->filterUndoFiles(destUrls);

    if (!filterUrls.isEmpty())
        WorkspaceHelper::instance()->laterRequestSelectFiles(destUrls);
}

void WorkspaceEventReceiver::handleMoveToTrashFileResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    if (ok && !srcUrls.isEmpty())
        WorkspaceHelper::instance()->updateRootFile(srcUrls);
}

void WorkspaceEventReceiver::handleRenameFileResult(const quint64 windowId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(windowId)
    Q_UNUSED(errMsg)

    if (!ok || renamedUrls.isEmpty())
        return;

    WorkspaceHelper::instance()->laterRequestSelectFiles(renamedUrls.values());
}

void WorkspaceEventReceiver::handleFileUpdate(const QUrl &url)
{
    WorkspaceHelper::instance()->fileUpdate(url);
}

ItemRoles WorkspaceEventReceiver::handleCurrentSortRole(quint64 windowId)
{
    return WorkspaceHelper::instance()->sortRole(windowId);
}

QList<ItemRoles> WorkspaceEventReceiver::handleColumnRoles(quint64 windowId)
{
    return WorkspaceHelper::instance()->columnRoles(windowId);
}

QRectF WorkspaceEventReceiver::handleGetVisualGeometry(const quint64 windowID)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget)
        return workspaceWidget->viewVisibleGeometry();

    return QRectF(0, 0, 0, 0);
}

QRectF WorkspaceEventReceiver::handleGetViewItemRect(const quint64 windowID, const QUrl &url, const ItemRoles role)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget)
        return workspaceWidget->itemRect(url, role);

    return QRectF(0, 0, 0, 0);
}

void WorkspaceEventReceiver::handleSetDefaultViewMode(const QString &scheme, const ViewMode mode)
{
    WorkspaceHelper::instance()->setDefaultViewMode(scheme, mode);
}

ViewMode WorkspaceEventReceiver::handleGetDefaultViewMode(const QString &scheme)
{
    return WorkspaceHelper::instance()->findViewMode(scheme);
}

ViewMode WorkspaceEventReceiver::handleGetCurrentViewMode(const quint64 windowID)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);

    if (workspaceWidget)
        return workspaceWidget->currentViewMode();

    return ViewMode::kNoneMode;
}

void WorkspaceEventReceiver::handleRegisterFileView(const QString &scheme)
{
    WorkspaceHelper::instance()->registerFileView(scheme);
}

void WorkspaceEventReceiver::handleRegisterMenuScene(const QString &scheme, const QString &scene)
{
    WorkspaceHelper::instance()->setWorkspaceMenuScene(scheme, scene);
}

QString WorkspaceEventReceiver::handleFindMenuScene(const QString &scheme)
{
    return WorkspaceHelper::instance()->findMenuScene(scheme);
}

void WorkspaceEventReceiver::handleRegisterCustomTopWidget(const QVariantMap &dataMap)
{
    CustomTopWidgetInfo info(dataMap);
    if (WorkspaceHelper::instance()->isRegistedTopWidget(info.scheme)) {
        fmWarning() << "custom top widget sechme " << info.scheme << "has been resigtered!";
    }

    WorkspaceHelper::instance()->registerTopWidgetCreator(info.scheme, [=]() {
        CustomTopWidgetInterface *interface {
            new CustomTopWidgetInterface
        };
        interface->setKeepShow(info.keepShow);
        interface->registeCreateTopWidgetCallback(info.createTopWidgetCb);
        interface->registeCreateTopWidgetCallback(info.showTopWidgetCb);
        return interface;
    });
}

bool WorkspaceEventReceiver::handleGetCustomTopWidgetVisible(const quint64 windowID, const QString &scheme)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget) {
        return workspaceWidget->getCustomTopWidgetVisible(scheme);
    }
    return false;
}

bool WorkspaceEventReceiver::handleCheckSchemeViewIsFileView(const QString &scheme)
{
    return WorkspaceHelper::instance()->registeredFileView(scheme);
}

QList<QUrl> WorkspaceEventReceiver::handleGetSelectedUrls(const quint64 windowID)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget) {
        auto view = workspaceWidget->currentViewPtr();
        if (view)
            return view->selectedUrlList();
        else
            return {};
    }

    return {};
}

void WorkspaceEventReceiver::handleSetCustomFilterData(quint64 windowID, const QUrl &url, const QVariant &data)
{
    WorkspaceHelper::instance()->setFilterData(windowID, url, data);
}

void WorkspaceEventReceiver::handleSetCustomFilterCallback(quint64 windowID, const QUrl &url, const QVariant callback)
{
    auto filterCallback = DPF_NAMESPACE::paramGenerator<FileViewFilterCallback>(callback);
    WorkspaceHelper::instance()->setFilterCallback(windowID, url, filterCallback);
}

bool WorkspaceEventReceiver::handleRegisterRoutePrehandle(const QString &scheme, const FileViewRoutePrehaldler &prehandler)
{
    return WorkspaceHelper::instance()->reigsterViewRoutePrehandler(scheme, prehandler);
}

void WorkspaceEventReceiver::handleRegisterDataCache(const QString &scheme)
{
    //    FileModelManager::instance()->registerDataCache(scheme);
}

void WorkspaceEventReceiver::handleSetAlwaysOpenInCurrentWindow(const quint64 windowID)
{
    WorkspaceHelper::instance()->setAlwaysOpenInCurrentWindow(windowID);
}

