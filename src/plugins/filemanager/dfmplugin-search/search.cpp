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
#include "utils/custommanager.h"
#include "fileinfo/searchfileinfo.h"
#include "iterator/searchdiriterator.h"
#include "watcher/searchfilewatcher.h"
#include "topwidget/advancesearchbar.h"
#include "menus/searchmenuscene.h"
#include "searchmanager/searchmanager.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/filemanager/workspace/workspaceservice.h"

#include "dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

Q_DECLARE_METATYPE(QList<QVariantMap> *);

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

    bindEvents();

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
    QVariantMap property;
    property["Property_Key_KeepAddressBar"] = true;
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", SearchHelper::scheme(), property);
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

void Search::bindEvents()
{
    // hook events
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomColumnRoles",
                            SearchHelper::instance(), &SearchHelper::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleDisplayName",
                            SearchHelper::instance(), &SearchHelper::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleData",
                            SearchHelper::instance(), &SearchHelper::customRoleData);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                            SearchHelper::instance(), &SearchHelper::blockPaste);

    // subscribe signal events
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Search_Start",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Search_Stop",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleStopSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_FilterView_Show",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleShowAdvanceSearchBar);
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl,
                                   SearchEventReceiverIns, &SearchEventReceiver::handleUrlChanged);

    // connect self slot events
    static constexpr auto selfSpace { DPF_MACRO_TO_STR(DPSEARCH_NAMESPACE) };
    dpfSlotChannel->connect(selfSpace, "slot_Custom_Register",
                            CustomManager::instance(), &CustomManager::registerCustomInfo);
    dpfSlotChannel->connect(selfSpace, "slot_Custom_IsDisableSearch",
                            CustomManager::instance(), &CustomManager::isDisableSearch);
    dpfSlotChannel->connect(selfSpace, "slot_Custom_RedirectedPath",
                            CustomManager::instance(), &CustomManager::redirectedPath);
}

}
