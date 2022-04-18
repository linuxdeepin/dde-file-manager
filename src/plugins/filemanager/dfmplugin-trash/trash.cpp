/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#include "trash.h"
#include "trashfileinfo.h"
#include "trashdiriterator.h"
#include "trashfilewatcher.h"
#include "utils/trashhelper.h"
#include "utils/trashfilehelper.h"
#include "menus/trashmenu.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/menu/menuservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

void Trash::initialize()
{
    UrlRoute::regScheme(TrashHelper::scheme(), "/", TrashHelper::icon(), true, tr("Trash"));
    InfoFactory::regClass<TrashFileInfo>(TrashHelper::scheme());
    WatcherFactory::regClass<TrashFileWatcher>(TrashHelper::scheme());
    DirIteratorFactory::regClass<TrashDirIterator>(TrashHelper::scheme());
    MenuService::regClass<TrashMenu>(TrashScene::kTrashMenu);
    connect(TrashHelper::winServIns(), &WindowsService::windowOpened, this, &Trash::onWindowOpened, Qt::DirectConnection);
    connect(&dpfInstance.listener(), &dpf::Listener::pluginsInitialized, this, &Trash::onAllPluginsInitialized, Qt::DirectConnection);
}

bool Trash::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag Trash::stop()
{
    return kSync;
}

void Trash::onWindowOpened(quint64 windId)
{
    auto window = TrashHelper::winServIns()->findWindowById(windId);
    Q_ASSERT_X(window, "Trash", "Cannot find window by id");
    if (window->titleBar())
        regTrashCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Trash::regTrashCrumbToTitleBar, Qt::DirectConnection);

    if (window->sideBar())
        installToSideBar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { installToSideBar(); }, Qt::DirectConnection);
}

void Trash::regTrashCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        TitleBar::CustomCrumbInfo info;
        info.scheme = TrashHelper::scheme();
        info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == TrashHelper::scheme(); };
        TrashHelper::titleServIns()->addCustomCrumbar(info);
    });
}

void Trash::onAllPluginsInitialized()
{
    addCustomTopWidget();
    addFileOperations();
}

void Trash::installToSideBar()
{
    SideBar::ItemInfo item;
    item.group = SideBar::DefaultGroup::kCommon;
    item.url = TrashHelper::rootUrl();
    item.iconName = TrashHelper::icon().name();
    item.text = tr("Trash");
    item.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    item.contextMenuCb = TrashHelper::contenxtMenuHandle;
    TrashHelper::sideBarServIns()->addItem(item);
}

void Trash::addFileOperations()
{
    TrashHelper::workspaceServIns()->addScheme(TrashHelper::scheme());
    WorkspaceService::service()->setWorkspaceMenuScene(TrashHelper::scheme(), TrashScene::kTrashMenu);
    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &TrashFileHelper::openFilesHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &TrashFileHelper::writeToClipBoardHandle;
    fileOpeationsHandle->moveToTash = &TrashFileHelper::moveToTrashHandle;
    fileOpeationsHandle->deletes = &TrashFileHelper::deletesHandle;
    fileOpeationsHandle->copy = &TrashFileHelper::copyHandle;
    fileOpeationsHandle->cut = &TrashFileHelper::cutHandle;
    TrashHelper::fileOperationsServIns()->registerOperations(TrashHelper::scheme(), fileOpeationsHandle);
}

void Trash::addCustomTopWidget()
{
    Workspace::CustomTopWidgetInfo info;
    info.scheme = TrashHelper::scheme();
    info.createTopWidgetCb = TrashHelper::createEmptyTrashTopWidget;
    info.showTopWidgetCb = TrashHelper::showTopWidget;
    TrashHelper::workspaceServIns()->addCustomTopWidget(info);
}
