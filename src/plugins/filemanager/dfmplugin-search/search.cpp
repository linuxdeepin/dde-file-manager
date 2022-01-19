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

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"

#include "services/filemanager/search/searchservice.h"
#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/workspace/workspaceservice.h"

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

namespace GlobalPrivate {
static WorkspaceService *workspaceService { nullptr };
}   // namespace GlobalPrivate

void Search::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(SearchService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    UrlRoute::regScheme(SearchHelper::scheme(), "/", {}, true, "");
    //注册Scheme为"search"的扩展的文件信息
    InfoFactory::regClass<SearchFileInfo>(SearchHelper::scheme());
    DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
}

bool Search::start()
{
    dpfInstance.eventDispatcher().subscribe(TitleBar::EventType::kDoSearch,
                                            SearchEventReceiverIns,
                                            &SearchEventReceiver::hadleSearch);

    auto &ctx = dpfInstance.serviceContext();
    GlobalPrivate::workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());

    if (!GlobalPrivate::workspaceService) {
        qCritical() << "Failed, init workspace \"workspaceService\" is empty";
        abort();
    }
    GlobalPrivate::workspaceService->addScheme(SearchHelper::scheme());

    return true;
}

dpf::Plugin::ShutdownFlag Search::stop()
{
    return kSync;
}

DPSEARCH_END_NAMESPACE
