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

#include "shares.h"

#include "utils/shareutils.h"
#include "fileinfo/sharefileinfo.h"
#include "iterator/shareiterator.h"
#include "watcher/sharewatcher.h"
#include "menu/sharemenu.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/sidebar/sidebar_defines.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/common/menu/menuservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

DPSHARES_USE_NAMESPACE

void Shares::initialize()
{
    DFMBASE_USE_NAMESPACE
    UrlRoute::regScheme(ShareUtils::scheme(), "/", ShareUtils::icon(), true, tr("My Shares"));

    InfoFactory::regClass<ShareFileInfo>(ShareUtils::scheme());
    DirIteratorFactory::regClass<ShareIterator>(ShareUtils::scheme());
    WacherFactory::regClass<ShareWatcher>(ShareUtils::scheme());
    DSC_NAMESPACE::MenuService::regClass<ShareMenu>(ShareScene::kShareScene);

    DSB_FM_USE_NAMESPACE
    connect(WindowsService::service(), &WindowsService::windowCreated, this, &Shares::onWindowCreated, Qt::DirectConnection);
    connect(WindowsService::service(), &WindowsService::windowOpened, this, &Shares::onWindowOpened, Qt::DirectConnection);
    connect(WindowsService::service(), &WindowsService::windowClosed, this, &Shares::onWindowClosed, Qt::DirectConnection);
}

bool Shares::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag Shares::stop()
{
    return dpf::Plugin::ShutdownFlag::kSync;
}

void Shares::onWindowCreated(quint64 winId)
{
}

void Shares::onWindowOpened(quint64 winId)
{
    DSB_FM_USE_NAMESPACE
    DFMBASE_USE_NAMESPACE
    auto window = WindowsService::service()->findWindowById(winId);

    if (window->sideBar())
        addToSidebar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { addToSidebar(); }, Qt::DirectConnection);
}

void Shares::onWindowClosed(quint64 winId)
{
}

void Shares::addToSidebar()
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
