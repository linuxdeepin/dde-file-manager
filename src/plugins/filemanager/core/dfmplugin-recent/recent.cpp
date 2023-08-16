// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recent.h"
#include "files/recentfileinfo.h"
#include "files/recentdiriterator.h"
#include "files/recentfilewatcher.h"
#include "utils/recentmanager.h"
#include "utils/recentfilehelper.h"
#include "menus/recentmenuscene.h"
#include "events/recenteventreceiver.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dfm_event_defines.h>

#include <dfm-base/utils/systempathutil.h>

using BasicViewFieldFunc = std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
Q_DECLARE_METATYPE(ContextMenuCallback);
Q_DECLARE_METATYPE(QList<QVariantMap> *);
Q_DECLARE_METATYPE(QList<QUrl> *);
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QFlags<QFileDevice::Permission>)
Q_DECLARE_METATYPE(BasicViewFieldFunc)
Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(Qt::DropAction *)

DFMBASE_USE_NAMESPACE

namespace dfmplugin_recent {

void Recent::initialize()
{
    UrlRoute::regScheme(RecentHelper::scheme(), "/", RecentHelper::icon(), true, tr("Recent"));
    //注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<RecentFileInfo>(RecentHelper::scheme());
    WatcherFactory::regClass<RecentFileWatcher>(RecentHelper::scheme());
    DirIteratorFactory::regClass<RecentDirIterator>(RecentHelper::scheme());

    followEvents();
    bindWindows();
    RecentEventReceiver::instance()->initConnect();
    RecentManager::instance();
}

bool Recent::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(RecentMenuCreator::name(), new RecentMenuCreator());

    QStringList &&filtes { "kFileSizeField", "kFileChangeTimeField", "kFileInterviewTimeField" };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                         RecentHelper::scheme(), filtes);

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", RecentHelper::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", RecentHelper::scheme(), RecentMenuCreator::name());

    addFileOperations();

    // events
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_SetPermission", RecentFileHelper::instance(), &RecentFileHelper::setPermissionHandle);

    return true;
}

void Recent::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "Recent", "Cannot find window by id");
    if (window->titleBar())
        regRecentCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Recent::regRecentCrumbToTitleBar, Qt::DirectConnection);
    if (window->sideBar())
        addRecentItem();
    else
        connect(
                window, &FileManagerWindow::sideBarInstallFinished, this, [this] { addRecentItem(); }, Qt::DirectConnection);
}

void Recent::addRecentItem()
{
    ContextMenuCallback contextMenuCb { RecentHelper::contenxtMenuHandle };
    const QString &nameKey = "Recent";
    const QString &displayName = tr("Recent");
    Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled };
    QVariantMap map {
        { "Property_Key_Group", "Group_Common" },
        { "Property_Key_DisplayName", displayName },
        { "Property_Key_Icon", RecentHelper::icon() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
        { "Property_Key_VisiableControl", "recent" },
        { "Property_Key_ReportName", nameKey }
    };

    QVariantMap bookmarkMap {
        { "Property_Key_NameKey", nameKey },
        { "Property_Key_DisplayName", displayName },
        { "Property_Key_Url", RecentHelper::rootUrl() },
        { "Property_Key_Index", -1 },
        { "Property_Key_IsDefaultItem", true },
        { "Property_Key_PluginItemData", map }
    };
    dpfSlotChannel->push("dfmplugin_bookmark", "slot_AddPluginItem", bookmarkMap);   // push item data to bookmark plugin as cache
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", RecentHelper::rootUrl(), map);
}

void Recent::followEvents()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles", RecentManager::instance(), &RecentManager::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName", RecentManager::instance(), &RecentManager::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Delegate_CheckTransparent", RecentManager::instance(), &RecentManager::isTransparent);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction", RecentManager::instance(), &RecentManager::checkDragDropAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDrop", RecentManager::instance(), &RecentManager::handleDropFiles);

    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", RecentManager::instance(), &RecentManager::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", RecentManager::instance(), &RecentManager::sepateTitlebarCrumb);

    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutToFile", RecentFileHelper::instance(), &RecentFileHelper::cutFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CopyFile", RecentFileHelper::instance(), &RecentFileHelper::copyFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", RecentFileHelper::instance(), &RecentFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_DeleteFile", RecentFileHelper::instance(), &RecentFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", RecentFileHelper::instance(), &RecentFileHelper::openFileInPlugin);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_LinkFile", RecentFileHelper::instance(), &RecentFileHelper::linkFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard", RecentFileHelper::instance(), &RecentFileHelper::writeUrlsToClipboard);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenInTerminal", RecentFileHelper::instance(), &RecentFileHelper::openFileInTerminal);
}

void Recent::bindWindows()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFileResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleFileCutResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrashResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleUpdateRecent);
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFilesResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleUpdateRecent);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleFileRenameResult);
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Recent::onWindowOpened, Qt::DirectConnection);
}

void Recent::regRecentCrumbToTitleBar()
{
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", RecentHelper::scheme(), QVariantMap {});
}

void Recent::addFileOperations()
{
    BasicViewFieldFunc func { RecentHelper::propetyExtensionFunc };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_BasicViewExtension_Register",
                         func, RecentHelper::scheme());
}
}
