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

#include "services/filemanager/search/searchservice.h"
#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

namespace GlobalPrivate {
static WindowsService *winServ { nullptr };
static WorkspaceService *workspaceServ { nullptr };
}   // namespace GlobalPrivate

void Search::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(SearchService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    UrlRoute::regScheme(SearchHelper::scheme(), "/", {}, true, tr("Search"));
    //注册Scheme为"search"的扩展的文件信息
    InfoFactory::regClass<SearchFileInfo>(SearchHelper::scheme());
    DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
    WacherFactory::regClass<SearchFileWatcher>(SearchHelper::scheme());

    GlobalPrivate::winServ = ctx.service<WindowsService>(WindowsService::name());
    Q_ASSERT(GlobalPrivate::winServ);
    connect(GlobalPrivate::winServ, &WindowsService::windowOpened, this, &Search::onWindowOpened, Qt::DirectConnection);
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
    auto window = GlobalPrivate::winServ->findWindowById(windId);
    Q_ASSERT_X(window, "Search", "Cannot find window by id");

    if (window->workSpace())
        regSearchToWorkspaceService();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, &Search::regSearchToWorkspaceService, Qt::DirectConnection);

    if (window->titleBar())
        regSearchCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Search::regSearchCrumbToTitleBar, Qt::DirectConnection);
}

void Search::regSearchCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto &ctx = dpfInstance.serviceContext();
        if (ctx.load(TitleBarService::name())) {
            auto titleBarServ = ctx.service<TitleBarService>(TitleBarService::name());
            TitleBar::CustomCrumbInfo info;
            info.scheme = SearchHelper::scheme();
            info.keepAddressBar = true;
            info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == SearchHelper::scheme(); };
            titleBarServ->addCustomCrumbar(info);
        }
    });
}

void Search::regSearchToWorkspaceService()
{
    auto &ctx = dpfInstance.serviceContext();
    GlobalPrivate::workspaceServ = ctx.service<WorkspaceService>(WorkspaceService::name());

    if (!GlobalPrivate::workspaceServ) {
        qCritical() << "Failed, init workspace \"workspaceService\" is empty";
        abort();
    }
    GlobalPrivate::workspaceServ->addScheme(SearchHelper::scheme());

    Workspace::CustomTopWidgetInfo info;
    info.scheme = SearchHelper::scheme();
    info.keepShow = false;
    info.createTopWidgetCb = []() { return new AdvanceSearchBar(); };
    GlobalPrivate::workspaceServ->addCustomTopWidget(info);
}

DPSEARCH_END_NAMESPACE
