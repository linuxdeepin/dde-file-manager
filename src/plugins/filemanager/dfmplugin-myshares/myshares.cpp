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
#include "fileinfo/sharefileinfo.h"
#include "iterator/shareiterator.h"
#include "watcher/sharewatcher.h"
#include "menu/mysharemenuscene.h"
#include "events/shareeventscaller.h"
#include "events/shareeventhelper.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/sidebar/sidebar_defines.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/search/searchservice.h"
#include "services/common/usershare/usershareservice.h"
#include "services/common/fileoperations/fileoperations_defines.h"
#include "services/common/fileoperations/fileoperationsservice.h"
#include "services/common/delegate/delegateservice.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_myshares;

void MyShares::initialize()
{
    DFMBASE_USE_NAMESPACE
    DSC_USE_NAMESPACE
    UrlRoute::regScheme(ShareUtils::scheme(), "/", ShareUtils::icon(), true, tr("My Shares"));

    InfoFactory::regClass<ShareFileInfo>(ShareUtils::scheme());
    DirIteratorFactory::regClass<ShareIterator>(ShareUtils::scheme());
    WatcherFactory::regClass<ShareWatcher>(ShareUtils::scheme());
    dfmplugin_menu_util::menuSceneRegisterScene(MyShareMenuCreator::name(), new MyShareMenuCreator);
    claimSubScene("SortAndDisplayMenu");   // using workspace's SortAndDisplayAsMenu

    DSB_FM_USE_NAMESPACE
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this, &MyShares::onWindowCreated, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &MyShares::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &MyShares::onWindowClosed, Qt::DirectConnection);

    UserShareService::service();   // for loading shares.
    connect(UserShareService::service(), &UserShareService::shareAdded, this, [this] { this->addToSidebar(); }, Qt::DirectConnection);
    connect(UserShareService::service(), &UserShareService::shareRemoved, this, [] {
        if (UserShareService::service()->shareInfos().count() == 0)
            SideBarService::service()->removeItem(ShareUtils::rootUrl());
    },
            Qt::DirectConnection);
}

bool MyShares::start()
{
    DSB_FM_USE_NAMESPACE

    WorkspaceService::service()->addScheme(ShareUtils::scheme());
    WorkspaceService::service()->setWorkspaceMenuScene(ShareUtils::scheme(), MyShareMenuCreator::name());
    FileOperationsFunctions fileOpeationsHandle(new DSC_NAMESPACE::FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = [](quint64 winId, QList<QUrl> urls, QString *) {
        ShareEventsCaller::sendOpenDirs(winId, urls, ShareEventsCaller::OpenMode::kOpenInCurrentWindow);
        return true;
    };
    DSC_NAMESPACE::FileOperationsService::service()->registerOperations(ShareUtils::scheme(), fileOpeationsHandle);

    delegateServIns->registerUrlTransform(ShareUtils::scheme(), [](QUrl in) {
        in.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
        return in;
    });

    hookEvent();

    return true;
}

dpf::Plugin::ShutdownFlag MyShares::stop()
{
    return dpf::Plugin::ShutdownFlag::kSync;
}

void MyShares::onWindowCreated(quint64 winId)
{
}

void MyShares::onWindowOpened(quint64 winId)
{
    DSB_FM_USE_NAMESPACE
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

void MyShares::onWindowClosed(quint64 winId)
{
}

void MyShares::addToSidebar()
{
    DSC_USE_NAMESPACE
    if (UserShareService::service()->shareInfos().count() == 0)
        return;

    DSB_FM_USE_NAMESPACE
    SideBar::ItemInfo shareEntry;
    shareEntry.group = SideBar::DefaultGroup::kNetwork;
    shareEntry.iconName = ShareUtils::icon().name();
    if (!shareEntry.iconName.endsWith("-symbolic"))
        shareEntry.iconName.append("-symbolic");
    shareEntry.text = ShareUtils::displayName();
    shareEntry.url = ShareUtils::rootUrl();
    shareEntry.flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    SideBarService::service()->addItem(shareEntry);
}

void MyShares::regMyShareToSearch()
{
    DSB_FM_USE_NAMESPACE
    Search::CustomSearchInfo info;
    info.scheme = ShareUtils::scheme();
    info.isDisableSearch = true;
    SearchService::service()->regCustomSearchInfo(info);
}

void MyShares::claimSubScene(const QString &scene)
{
    if (dfmplugin_menu_util::menuSceneContains(scene)) {
        dfmplugin_menu_util::menuSceneBind(scene, MyShareMenuCreator::name());
    } else {
        //todo(xst) menu
//        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &addedScene) {
//            if (scene == addedScene) {
//                MenuService::service()->bind(scene, MyShareMenuCreator::name());
//                MenuService::service()->disconnect(this);
//            }
//        },
//                Qt::DirectConnection);
    }
}

void MyShares::hookEvent()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles", ShareEventHelper::instance(), &ShareEventHelper::blockDelete);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_MoveToTrash", ShareEventHelper::instance(), &ShareEventHelper::blockMoveToTrash);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", ShareEventHelper::instance(), &ShareEventHelper::blockPaste);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_SendOpenWindow", ShareEventHelper::instance(), &ShareEventHelper::hookSendOpenWindow);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_SendChangeCurrentUrl", ShareEventHelper::instance(), &ShareEventHelper::hookSendChangeCurrentUrl);
}
