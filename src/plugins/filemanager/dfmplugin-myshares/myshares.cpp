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
#include "menu/sharemenu.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/sidebar/sidebar_defines.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/common/menu/menuservice.h"
#include "services/common/usershare/usershareservice.h"
#include "services/common/fileoperations/fileoperations_defines.h"
#include "services/common/fileoperations/fileoperationsservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

DPMYSHARES_USE_NAMESPACE

void MyShares::initialize()
{
    DFMBASE_USE_NAMESPACE
    DSC_USE_NAMESPACE
    UrlRoute::regScheme(ShareUtils::scheme(), "/", ShareUtils::icon(), true, tr("My Shares"));

    InfoFactory::regClass<ShareFileInfo>(ShareUtils::scheme());
    DirIteratorFactory::regClass<ShareIterator>(ShareUtils::scheme());
    WacherFactory::regClass<ShareWatcher>(ShareUtils::scheme());
    MenuService::regClass<ShareMenu>(ShareScene::kShareScene);

    DSB_FM_USE_NAMESPACE
    connect(WindowsService::service(), &WindowsService::windowCreated, this, &MyShares::onWindowCreated, Qt::DirectConnection);
    connect(WindowsService::service(), &WindowsService::windowOpened, this, &MyShares::onWindowOpened, Qt::DirectConnection);
    connect(WindowsService::service(), &WindowsService::windowClosed, this, &MyShares::onWindowClosed, Qt::DirectConnection);

    UserShareService::service();   // for loading shares.
}

bool MyShares::start()
{
    DSB_FM_USE_NAMESPACE

    WorkspaceService::service()->addScheme(ShareUtils::scheme());
    WorkspaceService::service()->setWorkspaceMenuScene(ShareUtils::scheme(), ShareScene::kShareScene);
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
    auto window = WindowsService::service()->findWindowById(winId);

    if (window->sideBar())
        addToSidebar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { addToSidebar(); }, Qt::DirectConnection);
}

void MyShares::onWindowClosed(quint64 winId)
{
}

void MyShares::addToSidebar()
{
    DSB_FM_USE_NAMESPACE
    SideBar::ItemInfo shareEntry;
    shareEntry.group = SideBar::DefaultGroup::kNetwork;
    shareEntry.iconName = ShareUtils::icon().name();
    if (!shareEntry.iconName.endsWith("-symbolic"))
        shareEntry.iconName.append("-symbolic");
    shareEntry.text = ShareUtils::displayName();
    shareEntry.url = ShareUtils::rootUrl();
    shareEntry.flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    SideBarService::service()->addItem(shareEntry);
}
