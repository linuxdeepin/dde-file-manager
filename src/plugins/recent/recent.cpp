/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "recenteventrecver.h"
#include "recentrootfileinfo.h"

#include "windowservice/windowservice.h"
#include "windowservice/browseview.h"
#include "applicationservice/application.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/abstractfilewatcher.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/localfile/localfileinfo.h"
#include "dfm-base/localfile/localdiriterator.h"
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"
#include "dfm-base/widgets/dfmfileview/fileview.h"

#include "dfm-framework/lifecycle/plugincontext.h"

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

//DFM_USE_NAMESPACE

void Recent::initialize()
{
    QString recentScheme{"recent"};

    RecentUtil::initRecentSubSystem();
    //注册路由
    UrlRoute::schemeMapRoot(recentScheme,"/",QIcon(),true);
    //注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::instance().regClass<RecentRootFileInfo>("recent");
    //    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("file");
    //    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("file");
    //    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("file");
    BrowseWidgetFactory::instance().regClass<RecentBrowseView>(recentScheme);
}

bool Recent::start(QSharedPointer<dpf::PluginContext> context)
{
    Q_UNUSED(context)

    SaveBlock::windowServiceTemp = context->service<WindowService>("WindowService");

    qCCritical(RecentPlugin) << context->services() << SaveBlock::windowServiceTemp;

    return true;
}

dpf::Plugin::ShutdownFlag Recent::stop() {
    return Synch;
}
