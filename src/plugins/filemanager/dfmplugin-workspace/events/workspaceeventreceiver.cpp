// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspaceeventreceiver.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"
#include "utils/filedatamanager.h"
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
    fmDebug() << "WorkspaceEventReceiver: Destructor called, unsubscribing from events";
    dpfSignalDispatcher->unsubscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged",
                                     WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged);
}

WorkspaceEventReceiver *WorkspaceEventReceiver::instance()
{
    static WorkspaceEventReceiver receiver;
    return &receiver;
}

void WorkspaceEventReceiver::initConnection()
{
    fmDebug() << "WorkspaceEventReceiver: Initializing event connections";

    // signal event
    dpfSignalDispatcher->subscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged",
                                   WorkspaceHelper::instance(), &WorkspaceHelper::trashStateChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Tab_Created",
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTabCreated);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Tab_Removed",
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTabRemoved);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Tab_Changed",
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTabChanged);

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
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_RegisterFocusFileViewDisabled",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterFocusFileViewDisabled);

    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_SetCustomViewProperty",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetCustomViewProperty);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetVisualGeometry",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetVisualGeometry);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetViewItemRect",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetViewItemRect);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetCurrentViewMode",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetCurrentViewMode);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_GetDefaultViewMode",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleGetDefaultViewMode);
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
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_CurrentGroupRole",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleCurrentGroupRole);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_SetGroup",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleSetGroup);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_RegisterDataCache",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterDataCache);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_View_AboutToChangeViewWidth",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleAboutToChangeViewWidth);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Model_RegisterLoadStrategy",
                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleRegisterLoadStrategy);

    dpfSignalDispatcher->subscribe(GlobalEventType::kSwitchViewMode,
                                   WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTileBarSwitchModeTriggered);
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

    fmDebug() << "WorkspaceEventReceiver: Event connections initialized successfully";
}

void WorkspaceEventReceiver::handleTileBarSwitchModeTriggered(quint64 windowId, int mode)
{
    fmDebug() << "WorkspaceEventReceiver: Handling tile bar switch mode triggered - window ID:" << windowId << "mode:" << mode;
    WorkspaceHelper::instance()->switchViewMode(windowId, mode);
}

void WorkspaceEventReceiver::handleShowCustomTopWidget(quint64 windowId, const QString &scheme, bool visible)
{
    fmDebug() << "WorkspaceEventReceiver: handling show custom top widget request for window" << windowId << "scheme" << scheme << "visible" << visible;
    WorkspaceHelper::instance()->setCustomTopWidgetVisible(windowId, scheme, visible);
}

void WorkspaceEventReceiver::handleSelectFiles(quint64 windowId, const QList<QUrl> &files)
{
    fmDebug() << "WorkspaceEventReceiver: handling select files request for window" << windowId << "with" << files.size() << "files";
    WorkspaceHelper::instance()->selectFiles(windowId, files);
}

void WorkspaceEventReceiver::handleSelectAll(quint64 windowId)
{
    fmDebug() << "WorkspaceEventReceiver: Handling select all request for window ID:" << windowId;
    WorkspaceHelper::instance()->selectAll(windowId);
}

void WorkspaceEventReceiver::handleReverseSelect(quint64 windowId)
{
    fmDebug() << "WorkspaceEventReceiver: Handling reverse select request for window ID:" << windowId;
    WorkspaceHelper::instance()->reverseSelect(windowId);
}

void WorkspaceEventReceiver::handleSetSort(quint64 windowId, ItemRoles role)
{
    fmDebug() << "WorkspaceEventReceiver: handling set sort request for window" << windowId << "role" << static_cast<int>(role);
    WorkspaceHelper::instance()->setSort(windowId, role);
}

ItemRoles WorkspaceEventReceiver::handleCurrentGroupRole(quint64 windowId)
{
    return WorkspaceHelper::instance()->groupRole(windowId);
}

void WorkspaceEventReceiver::handleSetGroup(quint64 windowId, ItemRoles role)
{
    fmDebug() << "WorkspaceEventReceiver: handling set group request for window" << windowId << "role" << static_cast<int>(role);
    WorkspaceHelper::instance()->setGroup(windowId, role);
}

void WorkspaceEventReceiver::handleSetSelectionMode(const quint64 windowId, const QAbstractItemView::SelectionMode mode)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set selection mode request for window" << windowId << "mode:" << static_cast<int>(mode);
    WorkspaceHelper::instance()->setSelectionMode(windowId, mode);
}

void WorkspaceEventReceiver::handleSetEnabledSelectionModes(const quint64 windowId, const QList<QAbstractItemView::SelectionMode> &modes)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set enabled selection modes request for window" << windowId << "modes count:" << modes.size();
    WorkspaceHelper::instance()->setEnabledSelectionModes(windowId, modes);
}

void WorkspaceEventReceiver::handleSetViewDragEnabled(const quint64 windowId, const bool enabled)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set view drag enabled request for window" << windowId << "enabled:" << enabled;
    WorkspaceHelper::instance()->setViewDragEnabled(windowId, enabled);
}

void WorkspaceEventReceiver::handleSetViewDragDropMode(const quint64 windowId, const QAbstractItemView::DragDropMode mode)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set view drag drop mode request for window" << windowId << "mode:" << static_cast<int>(mode);
    WorkspaceHelper::instance()->setViewDragDropMode(windowId, mode);
}

void WorkspaceEventReceiver::handleClosePersistentEditor(const quint64 windowId)
{
    fmDebug() << "WorkspaceEventReceiver: Handling close persistent editor request for window ID:" << windowId;
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
    fmDebug() << "WorkspaceEventReceiver: Handling set view filter request for window" << windowId << "filters:" << static_cast<int>(filters);
    WorkspaceHelper::instance()->setViewFilter(windowId, filters);
}

void WorkspaceEventReceiver::handleSetNameFilter(const quint64 windowId, const QStringList &filters)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set name filter request for window" << windowId << "filters count:" << filters.size();
    WorkspaceHelper::instance()->setNameFilter(windowId, filters);
}

void WorkspaceEventReceiver::handleSetReadOnly(const quint64 windowId, const bool readOnly)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set read only request for window" << windowId << "readOnly:" << readOnly;
    WorkspaceHelper::instance()->setReadOnly(windowId, readOnly);
}

void WorkspaceEventReceiver::handlePasteFileResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(srcUrls)
    Q_UNUSED(ok)
    Q_UNUSED(errMsg)

    // if paste files from revocation operate, these files should not be selected.
    QList<QUrl> filterUrls = WorkspaceHelper::instance()->filterUndoFiles(destUrls);
    fmDebug() << "WorkspaceEventReceiver: Filtered URLs count:" << filterUrls.size() << "from" << destUrls.size() << "total";

    if (!filterUrls.isEmpty()) {
        fmDebug() << "WorkspaceEventReceiver: Requesting selection of" << filterUrls.size() << "filtered files";
        WorkspaceHelper::instance()->laterRequestSelectFiles(destUrls);
    }
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

    if (!ok || renamedUrls.isEmpty()) {
        fmDebug() << "WorkspaceEventReceiver: No files to select after rename";
        return;
    }

    WorkspaceHelper::instance()->laterRequestSelectFiles(renamedUrls.values());
}

void WorkspaceEventReceiver::handleFileUpdate(const QUrl &url)
{
    fmDebug() << "WorkspaceEventReceiver: handling file update request for URL" << url.toString();
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

    fmWarning() << "WorkspaceEventReceiver: cannot find workspace widget for window ID" << windowID;
    return QRectF(0, 0, 0, 0);
}

QRectF WorkspaceEventReceiver::handleGetViewItemRect(const quint64 windowID, const QUrl &url, const ItemRoles role)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget)
        return workspaceWidget->itemRect(url, role);

    fmWarning() << "WorkspaceEventReceiver: cannot find workspace widget for window ID" << windowID << "URL" << url.toString();
    return QRectF(0, 0, 0, 0);
}

void WorkspaceEventReceiver::handleSetCustomViewProperty(const QString &scheme, const QVariantMap &properties)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set custom view property request for scheme:" << scheme << "properties count:" << properties.size();
    WorkspaceHelper::instance()->registerCustomViewProperty(scheme, properties);
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

    fmWarning() << "WorkspaceEventReceiver: cannot find workspace widget for window ID" << windowID;
    return ViewMode::kNoneMode;
}

void WorkspaceEventReceiver::handleRegisterFileView(const QString &scheme)
{
    fmDebug() << "WorkspaceEventReceiver: Handling register file view request for scheme:" << scheme;
    WorkspaceHelper::instance()->registerFileView(scheme);
}

void WorkspaceEventReceiver::handleRegisterMenuScene(const QString &scheme, const QString &scene)
{
    fmDebug() << "WorkspaceEventReceiver: Handling register menu scene request for scheme:" << scheme << "scene:" << scene;
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

    fmDebug() << "WorkspaceEventReceiver: Registering custom top widget for scheme:" << info.scheme;
    WorkspaceHelper::instance()->registerTopWidgetCreator(info.scheme, [=]() {
        CustomTopWidgetInterface *interface {
            new CustomTopWidgetInterface
        };
        interface->setKeepShow(info.keepShow);
        interface->setKeepTop(info.keepTop);
        interface->registeCreateTopWidgetCallback(info.createTopWidgetCb);
        interface->registeCreateTopWidgetCallback(info.showTopWidgetCb);
        fmDebug() << "WorkspaceEventReceiver: Created custom top widget interface for scheme:" << info.scheme;
        return interface;
    });
}

bool WorkspaceEventReceiver::handleGetCustomTopWidgetVisible(const quint64 windowID, const QString &scheme)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget) {
        return workspaceWidget->getCustomTopWidgetVisible(scheme);
    }

    fmWarning() << "WorkspaceEventReceiver: Cannot find workspace widget for window ID" << windowID;
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
        auto view = workspaceWidget->currentView();
        if (view) {
            return view->selectedUrlList();
        } else {
            fmWarning() << "WorkspaceEventReceiver: No current view found for window ID:" << windowID;
            return {};
        }
    }

    fmWarning() << "WorkspaceEventReceiver: Cannot find workspace widget for window ID:" << windowID;
    return {};
}

void WorkspaceEventReceiver::handleSetCustomFilterData(quint64 windowID, const QUrl &url, const QVariant &data)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set custom filter data request for window ID:" << windowID << "URL:" << url.toString() << "data valid:" << data.isValid();
    WorkspaceHelper::instance()->setFilterData(windowID, url, data);
}

void WorkspaceEventReceiver::handleSetCustomFilterCallback(quint64 windowID, const QUrl &url, const QVariant callback)
{
    fmDebug() << "WorkspaceEventReceiver: Handling set custom filter callback request for window ID:" << windowID << "URL:" << url.toString() << "callback valid:" << callback.isValid();

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
    fmDebug() << "WorkspaceEventReceiver: Handling set always open in current window request for window ID:" << windowID;
    WorkspaceHelper::instance()->setAlwaysOpenInCurrentWindow(windowID);
}

void WorkspaceEventReceiver::handleAboutToChangeViewWidth(const quint64 windowID, int deltaWidth)
{
    fmDebug() << "WorkspaceEventReceiver: Handling about to change view width request for window ID:" << windowID << "delta width:" << deltaWidth;
    WorkspaceHelper::instance()->aboutToChangeViewWidth(windowID, deltaWidth);
}

void WorkspaceEventReceiver::handleTabCreated(const quint64 windowId, const QString &uniqueId)
{
    fmDebug() << "WorkspaceEventReceiver: Handling tab created event for window ID:" << windowId << "unique ID:" << uniqueId;

    auto workspace = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowId);
    if (workspace) {
        workspace->createNewPage(uniqueId);
        fmDebug() << "WorkspaceEventReceiver: Created new page for window" << windowId << "unique ID" << uniqueId;
    }
}

void WorkspaceEventReceiver::handleTabRemoved(const quint64 windowId, const QString &removedId, const QString &nextId)
{
    fmDebug() << "WorkspaceEventReceiver: Handling tab removed event for window ID:" << windowId << "removed ID:" << removedId << "next ID:" << nextId;

    auto workspace = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowId);
    if (workspace) {
        workspace->removePage(removedId, nextId);
        fmDebug() << "WorkspaceEventReceiver: Removed page for window" << windowId << "removed ID" << removedId;
    }
}

void WorkspaceEventReceiver::handleTabChanged(const quint64 windowId, const QString &uniqueId)
{
    fmDebug() << "WorkspaceEventReceiver: Handling tab changed event for window ID:" << windowId << "unique ID:" << uniqueId;

    auto workspace = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowId);
    if (workspace) {
        workspace->setCurrentPage(uniqueId);
        fmDebug() << "WorkspaceEventReceiver: Set current page for window" << windowId << "unique ID" << uniqueId;
    }
}

void WorkspaceEventReceiver::handleRegisterLoadStrategy(const QString &scheme, DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy strategy)
{
    fmDebug() << "WorkspaceEventReceiver: Handling register load strategy request for scheme:" << scheme << "strategy:" << static_cast<int>(strategy);
    WorkspaceHelper::instance()->registerLoadStrategy(scheme, strategy);
}

void WorkspaceEventReceiver::handleRegisterFocusFileViewDisabled(const QString &scheme)
{
    fmDebug() << "WorkspaceEventReceiver: Handling register focus file view disabled request for scheme:" << scheme;
    WorkspaceHelper::instance()->registerFocusFileViewDisabled(scheme);
}
