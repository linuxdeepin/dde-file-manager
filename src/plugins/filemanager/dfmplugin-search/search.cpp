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
#include "menus/searchmenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/search/searchservice.h"

#include "dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
namespace dfmplugin_search {

void Search::initialize()
{
    UrlRoute::regScheme(SearchHelper::scheme(), "/", {}, true, tr("Search"));
    //注册Scheme为"search"的扩展的文件信息
    InfoFactory::regClass<SearchFileInfo>(SearchHelper::scheme());
    DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
    WatcherFactory::regClass<SearchFileWatcher>(SearchHelper::scheme());

    subscribeEvent();

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Search::onWindowOpened, Qt::DirectConnection);
}

bool Search::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(SearchMenuCreator::name(), new SearchMenuCreator());
    return true;
}

dpf::Plugin::ShutdownFlag Search::stop()
{
    return kSync;
}

void Search::subscribeEvent()
{
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_StartSearch", SearchEventReceiverIns, &SearchEventReceiver::handleSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_StopSearch", SearchEventReceiverIns, &SearchEventReceiver::handleStopSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_ShowFilterView", SearchEventReceiverIns, &SearchEventReceiver::handleShowAdvanceSearchBar);
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl, SearchEventReceiverIns, &SearchEventReceiver::handleUrlChanged);

    followEvent();
}

void Search::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "Search", "Cannot find window by id");

    if (window->workSpace())
        regSearchToWorkspace();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, &Search::regSearchToWorkspace, Qt::DirectConnection);

    if (window->titleBar())
        regSearchCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Search::regSearchCrumbToTitleBar, Qt::DirectConnection);
}

void Search::regSearchCrumbToTitleBar()
{
    TitleBar::CustomCrumbInfo info;
    info.scheme = SearchHelper::scheme();
    info.keepAddressBar = true;
    info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == SearchHelper::scheme(); };
    TitleBarService::service()->addCustomCrumbar(info);
}

void Search::regSearchToWorkspace()
{
    WorkspaceService::service()->addScheme(SearchHelper::scheme());
    WorkspaceService::service()->setDefaultViewMode(SearchHelper::scheme(), Global::ViewMode::kListMode);
    WorkspaceService::service()->setWorkspaceMenuScene(SearchHelper::scheme(), SearchMenuCreator::name());

    Workspace::CustomTopWidgetInfo info;
    info.scheme = SearchHelper::scheme();
    info.keepShow = false;
    info.createTopWidgetCb = []() { return new AdvanceSearchBar(); };
    info.showTopWidgetCb = SearchHelper::showTopWidget;
    WorkspaceService::service()->addCustomTopWidget(info);
}

void Search::followEvent()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomColumnRoles", SearchHelper::instance(), &SearchHelper::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleDisplayName", SearchHelper::instance(), &SearchHelper::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleData", SearchHelper::instance(), &SearchHelper::customRoleData);

    // disable paste
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", SearchHelper::instance(), &SearchHelper::blockPaste);
}

}
