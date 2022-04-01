/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "search.h"
#include "events/searcheventreceiver.h"
#include "utils/searchhelper.h"
#include "fileinfo/searchfileinfo.h"
#include "iterator/searchdiriterator.h"
#include "watcher/searchfilewatcher.h"
#include "topwidget/advancesearchbar.h"
#include "menus/searchmenu.h"
#include "utils/searchfileoperations.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/search/searchservice.h"
#include "services/common/menu/menuservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm_global_defines.h"

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

void Search::initialize()
{
    UrlRoute::regScheme(SearchHelper::scheme(), "/", {}, true, tr("Search"));
    //注册Scheme为"search"的扩展的文件信息
    InfoFactory::regClass<SearchFileInfo>(SearchHelper::scheme());
    DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
    WatcherFactory::regClass<SearchFileWatcher>(SearchHelper::scheme());
    MenuService::regClass<SearchMenu>(SearchScene::kSearchMenu);

    connect(WindowsService::service(), &WindowsService::windowOpened, this, &Search::onWindowOpened, Qt::DirectConnection);
}

bool Search::start()
{
    subscribeEvent();
    return true;
}

dpf::Plugin::ShutdownFlag Search::stop()
{
    return kSync;
}

void Search::subscribeEvent()
{
    dpfInstance.eventDispatcher().subscribe(TitleBar::EventType::kDoSearch,
                                            SearchEventReceiverIns,
                                            &SearchEventReceiver::handleSearch);
    dpfInstance.eventDispatcher().subscribe(TitleBar::EventType::kStopSearch,
                                            SearchEventReceiverIns,
                                            &SearchEventReceiver::handleStopSearch);
    dpfInstance.eventDispatcher().subscribe(TitleBar::EventType::kShowFilterView,
                                            SearchEventReceiverIns,
                                            &SearchEventReceiver::handleShowAdvanceSearchBar);
}

void Search::onWindowOpened(quint64 windId)
{
    auto window = WindowsService::service()->findWindowById(windId);
    Q_ASSERT_X(window, "Search", "Cannot find window by id");

    if (window->workSpace())
        regSearchPlugin();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, &Search::regSearchPlugin, Qt::DirectConnection);

    if (window->titleBar())
        regSearchCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Search::regSearchCrumbToTitleBar, Qt::DirectConnection);
}

void Search::regSearchCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        TitleBar::CustomCrumbInfo info;
        info.scheme = SearchHelper::scheme();
        info.keepAddressBar = true;
        info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == SearchHelper::scheme(); };
        TitleBarService::service()->addCustomCrumbar(info);
    });
}

void Search::regSearchPlugin()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        regSearchToWorkspaceService();
        regSearchFileOperations();
    });
}

void Search::regSearchToWorkspaceService()
{
    WorkspaceService::service()->addScheme(SearchHelper::scheme());
    WorkspaceService::service()->setDefaultViewMode(SearchHelper::scheme(), Global::ViewMode::kListMode);
    WorkspaceService::service()->setWorkspaceMenuScene(SearchHelper::scheme(), SearchScene::kSearchMenu);

    Workspace::CustomTopWidgetInfo info;
    info.scheme = SearchHelper::scheme();
    info.keepShow = false;
    info.createTopWidgetCb = []() { return new AdvanceSearchBar(); };
    WorkspaceService::service()->addCustomTopWidget(info);
}

void Search::regSearchFileOperations()
{
    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &SearchFileOperations::openFilesHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &SearchFileOperations::writeToClipBoardHandle;
    fileOpeationsHandle->moveToTash = &SearchFileOperations::moveToTrashHandle;
    fileOpeationsHandle->deletes = &SearchFileOperations::deleteFilesHandle;
    fileOpeationsHandle->renameFile = &SearchFileOperations::renameFileHandle;
    fileOpeationsHandle->openInTerminal = &SearchFileOperations::openInTerminalHandle;

    FileOperationsService::service()->registerOperations(SearchHelper::scheme(), fileOpeationsHandle);
}

DPSEARCH_END_NAMESPACE
