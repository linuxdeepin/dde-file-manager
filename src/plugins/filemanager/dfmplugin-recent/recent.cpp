/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "recent.h"
#include "recentutil.h"
#include "recentbrowseview.h"
#include "recenteventreceiver.h"
#include "recentfileinfo.h"
#include "recentdiriterator.h"

#include "windowservice.h"
#include "window/browseview.h"   // TODO(zhangs): hide

#include "dfm-base/application/application.h"
#include "dfm-base/base/abstractfilewatcher.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/localfile/localfileinfo.h"
#include "dfm-base/localfile/localdiriterator.h"
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"
#include "dfm-base/widgets/dfmfileview/fileview.h"
#include "services/common/menu/menuservice.h"

#include <QListWidget>
#include <QListView>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QDockWidget>
#include <QStatusBar>
#include <QLabel>
#include <QFrame>
#include <QIcon>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSplitter>
#include <QDir>
#include <QSizePolicy>
#include <QToolButton>
#include <dfm-framework/framework.h>

//DFM_USE_NAMESPACE
DSC_USE_NAMESPACE

void Recent::initialize()
{
    QString recentScheme { "recent" };

    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(MenuService::name(), &errStr)) {
        dpfDebug() << errStr;
        abort();
    }

    RecentUtil::initRecentSubSystem();
    //注册路由
    QIcon recentIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kRecentPath));
    UrlRoute::regScheme(recentScheme, "/", recentIcon, true);
    //注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<RecentFileInfo>(recentScheme);
    //    FileDeviceFactory::regClass<>("recent");
    DirIteratorFactory::regClass<RecentDirIterator>(recentScheme);
    //    WacherFactory::regClass<AbstractFileWatcher>("recent");
    BrowseWidgetFactory::regClass<RecentBrowseView>(recentScheme);
    MenuService::regClass<AbstractFileMenu>(recentScheme);
}

bool Recent::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag Recent::stop()
{
    return kSync;
}
