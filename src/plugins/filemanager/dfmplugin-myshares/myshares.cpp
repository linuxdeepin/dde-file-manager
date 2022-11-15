/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "myshares.h"

#include "utils/shareutils.h"
#include "utils/sharefilehelper.h"
#include "fileinfo/sharefileinfo.h"
#include "iterator/shareiterator.h"
#include "watcher/sharewatcher.h"
#include "menu/mysharemenuscene.h"
#include "events/shareeventscaller.h"
#include "events/shareeventhelper.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/event/event.h>

#include <QMenu>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_myshares;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;

Q_DECLARE_METATYPE(ContextMenuCallback);
void MyShares::initialize()
{
    DFMBASE_USE_NAMESPACE
    UrlRoute::regScheme(ShareUtils::scheme(), "/", ShareUtils::icon(), true, tr("My Shares"));

    InfoFactory::regClass<ShareFileInfo>(ShareUtils::scheme());
    DirIteratorFactory::regClass<ShareIterator>(ShareUtils::scheme());
    WatcherFactory::regClass<ShareWatcher>(ShareUtils::scheme());
    dfmplugin_menu_util::menuSceneRegisterScene(MyShareMenuCreator::name(), new MyShareMenuCreator);
    beMySubScene("SortAndDisplayMenu");   // using workspace's SortAndDisplayAsMenu

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &MyShares::onWindowOpened, Qt::DirectConnection);
}

bool MyShares::start()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", ShareUtils::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", ShareUtils::scheme(), MyShareMenuCreator::name());

    dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareAdded", this, &MyShares::onShareAdded);
    dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareRemoved", this, &MyShares::onShareRemoved);

    hookEvent();

    return true;
}

void MyShares::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    QMenu *menu = new QMenu;
    auto newWindowAct = menu->addAction(QObject::tr("Open in new window"), [url]() { ShareEventsCaller::sendOpenWindow(url); });
    newWindowAct->setEnabled(bEnabled);

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        ShareEventsCaller::sendOpenTab(windowId, url);
    });

    newTabAct->setEnabled(bEnabled && ShareEventsCaller::sendCheckTabAddable(windowId));

    menu->exec(globalPos);
    delete menu;
}

void MyShares::onWindowOpened(quint64 winId)
{
    DFMBASE_USE_NAMESPACE
    auto window = FMWindowsIns.findWindowById(winId);

    if (window->sideBar())
        addToSidebar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { addToSidebar(); }, Qt::DirectConnection);

    if (window->titleBar())
        regMyShareToSearch();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, [this] { regMyShareToSearch(); }, Qt::DirectConnection);
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

void MyShares::hookEvent()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles", ShareEventHelper::instance(), &ShareEventHelper::blockDelete);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_MoveToTrash", ShareEventHelper::instance(), &ShareEventHelper::blockMoveToTrash);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", ShareEventHelper::instance(), &ShareEventHelper::blockPaste);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_SendOpenWindow", ShareEventHelper::instance(), &ShareEventHelper::hookSendOpenWindow);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_SendChangeCurrentUrl", ShareEventHelper::instance(), &ShareEventHelper::hookSendChangeCurrentUrl);
    dpfHookSequence->follow("dfmplugin_utils", "hook_UrlsTransform", ShareUtils::instance(), &ShareUtils::urlsToLocal);

    // file operation
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", ShareFileHelper::instance(), &ShareFileHelper::openFileInPlugin);
}
