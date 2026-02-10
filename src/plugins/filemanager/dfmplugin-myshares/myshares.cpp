// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "myshares.h"

#include "utils/shareutils.h"
#include "utils/sharefilehelper.h"
#include "fileinfo/sharefileinfo.h"
#include "iterator/shareiterator.h"
#include "watcher/sharewatcher.h"
#include "menu/mysharemenuscene.h"
#include "events/shareeventscaller.h"
#include "events/shareeventhelper.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <DMenu>

DWIDGET_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(ContextMenuCallback);

namespace dfmplugin_myshares {
DFM_LOG_REGISTER_CATEGORY(DPMYSHARES_NAMESPACE)

void MyShares::initialize()
{
    // TODO(zhangs): add feature for lazyload plugins depends
    auto dirsharePlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-dirshare") };
    if (dirsharePlugin && dirsharePlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        doInitialize();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-dirshare")
                        doInitialize();
                },
                Qt::DirectConnection);
    }
}

bool MyShares::start()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", ShareUtils::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", ShareUtils::scheme(), MyShareMenuCreator::name());

    return true;
}

void MyShares::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    DMenu *menu = new DMenu;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(menu), AcName::kAcSidebarShareMenu);
#endif
    auto newWindowAct = menu->addAction(QObject::tr("Open in new window"), [url]() { ShareEventsCaller::sendOpenWindow(url); });
    newWindowAct->setEnabled(bEnabled);

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        ShareEventsCaller::sendOpenTab(windowId, url);
    });

    newTabAct->setEnabled(bEnabled && ShareEventsCaller::sendCheckTabAddable(windowId));

    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_myshares", "signal_ReportLog_MenuData", act->text(), urls);
    }

    delete menu;
}

void MyShares::onWindowOpened(quint64 winId)
{
    DFMBASE_USE_NAMESPACE
    auto window = FMWindowsIns.findWindowById(winId);

    if (window->sideBar())
        addToSidebar();
    else
        connect(
                window, &FileManagerWindow::sideBarInstallFinished, this, [this] { addToSidebar(); }, Qt::DirectConnection);

    auto searchPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-search") };
    if (searchPlugin && searchPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        regMyShareToSearch();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-search")
                        regMyShareToSearch();
                },
                Qt::DirectConnection);
    }
}

void MyShares::onShareAdded(const QString &)
{
    addToSidebar();
}

void MyShares::onShareRemoved(const QString &)
{
    int count = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_AllShareInfos").value<ShareInfoList>().count();
    if (count == 0)
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", ShareUtils::rootUrl());
}

void MyShares::addToSidebar()
{
    int count = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_AllShareInfos").value<ShareInfoList>().count();
    if (count == 0)
        return;

    ContextMenuCallback contextMenuCb { MyShares::contenxtMenuHandle };
    Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren };
    QString iconName { ShareUtils::icon().name() };
    if (!iconName.endsWith("-symbolic"))
        iconName.append("-symbolic");
    QVariantMap map {
        { "Property_Key_Group", "Group_Network" },
        { "Property_Key_DisplayName", ShareUtils::displayName() },
        { "Property_Key_Icon", QIcon::fromTheme(iconName) },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_VisiableControl", "my_shares" },
        { "Property_Key_ReportName", "UserShare" },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) }
    };
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Insert", 1, ShareUtils::rootUrl(), map);
}

void MyShares::regMyShareToSearch()
{
    QVariantMap property;
    property["Property_Key_DisableSearch"] = true;
    dpfSlotChannel->push("dfmplugin_search", "slot_Custom_Register", ShareUtils::scheme(), property);
}

void MyShares::beMySubScene(const QString &scene)
{
    if (dfmplugin_menu_util::menuSceneContains(scene)) {
        dfmplugin_menu_util::menuSceneBind(scene, MyShareMenuCreator::name());
    } else {
        waitToBind << scene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &MyShares::beMySubOnAdded);
    }
}

void MyShares::beMySubOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &MyShares::beMySubOnAdded);
        beMySubScene(newScene);
    }
}

void MyShares::followEvents()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles", ShareEventHelper::instance(), &ShareEventHelper::blockDelete);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_MoveToTrash", ShareEventHelper::instance(), &ShareEventHelper::blockMoveToTrash);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", ShareEventHelper::instance(), &ShareEventHelper::blockPaste);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_SendOpenWindow", ShareEventHelper::instance(), &ShareEventHelper::hookSendOpenWindow);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_SendChangeCurrentUrl", ShareEventHelper::instance(), &ShareEventHelper::hookSendChangeCurrentUrl);

    // file operation
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", ShareFileHelper::instance(), &ShareFileHelper::openFileInPlugin);
}

void MyShares::bindWindows()
{
    DFMBASE_USE_NAMESPACE

    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &MyShares::onWindowOpened, Qt::DirectConnection);
}

void MyShares::doInitialize()
{
    DFMBASE_USE_NAMESPACE
    UrlRoute::regScheme(ShareUtils::scheme(), "/", ShareUtils::icon(), true, tr("My Shares"));

    InfoFactory::regClass<ShareFileInfo>(ShareUtils::scheme());
    DirIteratorFactory::regClass<ShareIterator>(ShareUtils::scheme());
    WatcherFactory::regClass<ShareWatcher>(ShareUtils::scheme());
    dfmplugin_menu_util::menuSceneRegisterScene(MyShareMenuCreator::name(), new MyShareMenuCreator);
    beMySubScene("SortAndDisplayMenu");   // using workspace's SortAndDisplayAsMenu
    beMySubScene("BookmarkMenu");

    dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareAdded", this, &MyShares::onShareAdded);
    dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareRemoved", this, &MyShares::onShareRemoved);

    QVariantMap property;
    property[ViewCustomKeys::kSupportTreeMode] = false;
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetCustomViewProperty", ShareUtils::scheme(), property);
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", ShareUtils::scheme(), property);

    followEvents();
    bindWindows();
}
}   // namespace dfmplugin_myshares
