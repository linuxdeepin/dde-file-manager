// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
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
DFMGLOBAL_USE_NAMESPACE

namespace dfmplugin_recent {
DFM_LOG_REGISTER_CATEGORY(DPRECENT_NAMESPACE)

void Recent::initialize()
{
    UrlRoute::regScheme(RecentHelper::scheme(), "/", RecentHelper::icon(), true, tr("Recent"));
    // 注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<RecentFileInfo>(RecentHelper::scheme());
    WatcherFactory::regClass<RecentFileWatcher>(RecentHelper::scheme());
    DirIteratorFactory::regClass<RecentDirIterator>(RecentHelper::scheme());

    followEvents();
    bindEvents();
    bindWindows();
    RecentManager::instance()->init();
}

bool Recent::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(RecentMenuCreator::name(), new RecentMenuCreator());

    QStringList &&filtes { "kFileSizeField", "kFileChangeTimeField", "kFileInterviewTimeField" };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                         RecentHelper::scheme(), filtes);

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", RecentHelper::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", RecentHelper::scheme(), RecentMenuCreator::name());

    QVariantMap property;
    property[ViewCustomKeys::kSupportTreeMode] = false;
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetCustomViewProperty", RecentHelper::scheme(), property);

    // add to property
    BasicViewFieldFunc func { RecentHelper::propetyExtensionFunc };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_BasicViewExtension_Register",
                         func, RecentHelper::scheme());

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
        regRecentItemToSideBar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &Recent::regRecentItemToSideBar, Qt::DirectConnection);
}

void Recent::updateRecentItemToSideBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        ContextMenuCallback contextMenuCb { RecentHelper::contenxtMenuHandle };
        Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled };
        QVariantMap map {
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
        };

        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", RecentHelper::rootUrl(), map);
    });
}

void Recent::followEvents()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles", RecentEventReceiver::instance(), &RecentEventReceiver::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName", RecentEventReceiver::instance(), &RecentEventReceiver::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Delegate_CheckTransparent", RecentEventReceiver::instance(), &RecentEventReceiver::isTransparent);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction", RecentEventReceiver::instance(), &RecentEventReceiver::checkDragDropAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDrop", RecentEventReceiver::instance(), &RecentEventReceiver::handleDropFiles);

    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", RecentEventReceiver::instance(), &RecentEventReceiver::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", RecentEventReceiver::instance(), &RecentEventReceiver::sepateTitlebarCrumb);
    dpfHookSequence->follow("dfmplugin_propertydialog", "hook_PropertyDialog_Disable", RecentEventReceiver::instance(), &RecentEventReceiver::handlePropertydialogDisable);

    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutToFile", RecentFileHelper::instance(), &RecentFileHelper::cutFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CopyFile", RecentFileHelper::instance(), &RecentFileHelper::copyFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", RecentFileHelper::instance(), &RecentFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_DeleteFile", RecentFileHelper::instance(), &RecentFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", RecentFileHelper::instance(), &RecentFileHelper::openFileInPlugin);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard", RecentFileHelper::instance(), &RecentFileHelper::writeUrlsToClipboard);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenInTerminal", RecentFileHelper::instance(), &RecentFileHelper::openFileInTerminal);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_SetPermission", RecentFileHelper::instance(), &RecentFileHelper::setPermissionHandle);
}

void Recent::bindEvents()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl, RecentEventReceiver::instance(), &RecentEventReceiver::handleWindowUrlChanged);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFileResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleFileCutResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrashResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleRemoveFilesResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFilesResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleRemoveFilesResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult, RecentEventReceiver::instance(), &RecentEventReceiver::handleFileRenameResult);
}

void Recent::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Recent::onWindowOpened, Qt::DirectConnection);
}

void Recent::regRecentCrumbToTitleBar()
{
    QVariantMap property;
    property[ViewCustomKeys::kSupportTreeMode] = false;
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", RecentHelper::scheme(), property);
}

void Recent::regRecentItemToSideBar()
{
    auto bookmarkPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-bookmark") };
    if (bookmarkPlugin && bookmarkPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        updateRecentItemToSideBar();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-bookmark")
                        updateRecentItemToSideBar();
                },
                Qt::DirectConnection);
    }
}

}
