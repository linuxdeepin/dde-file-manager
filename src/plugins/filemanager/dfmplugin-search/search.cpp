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
#include "searchbrowseview.h"
#include "searchlog.h"
#include "searchfileinfo.h"

#include "windowservice.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"
#include "dfm-base/widgets/dfmfileview/fileview.h"
#include "services/common/menu/menuservice.h"

#define SEARCH_SCHEME "search"

DSC_USE_NAMESPACE

void Search::initialize()
{
    //注册路由
    UrlRoute::regScheme(SEARCH_SCHEME, "/", {}, true);
    InfoFactory::regClass<SearchFileInfo>(SEARCH_SCHEME);
    //    DirIteratorFactory::regClass<SearchDirIterator>(SEARCH_SCHEME);
    //    WacherFactory::regClass<SearchFileWatcher>(SEARCH_SCHEME);
    BrowseWidgetFactory::regClass<SearchBrowseView>(SEARCH_SCHEME);
    MenuService::regClass<AbstractFileMenu>(SEARCH_SCHEME);
}

bool Search::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag Search::stop()
{
    return kSync;
}
