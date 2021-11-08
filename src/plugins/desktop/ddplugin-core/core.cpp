/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "core.h"
#include "screenproxydbus.h"
#include "screenproxyqt.h"
#include "backgrounddefault.h"

#include "services/common/menu/menuservice.h"

#include "wallpaperservice.h"
#include "screenservice.h"
#include "canvasservice.h"
#include "backgroundservice.h"

#include "dfm-base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/localfile/localfileinfo.h"
#include "dfm-base/localfile/localdiriterator.h"
#include "dfm-base/localfile/localfilewatcher.h"
#include "dfm-base/localfile/localfilemenu.h"
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"
#include "dfm-base/widgets/dfmfileview/fileview.h"

#include <dfm-framework/framework.h>

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
#include <QCoreApplication>
#include <QToolButton>
#include <QApplication>
#include <QScreen>

DSC_USE_NAMESPACE
DSB_D_USE_NAMESPACE

namespace GlobalPrivate {
    static QList<dfmbase::AbstractScreen*> screens{};
    static QList<dfmbase::AbstractBackground*> backgrounds{};
} // namespace GlobalPrivate

void registerAllService()
{
    ScreenService::regClass<ScreenProxyQt>(PlatformTypes::XCB);
    ScreenService::regClass<ScreenProxyDBus>(PlatformTypes::WAYLAND);
}

void registerFileSystem()
{
    UrlRoute::regScheme(SchemeTypes::FILE,"/");
    UrlRoute::regScheme(SchemeTypes::DESKTOP,
                        StandardPaths::location(StandardPaths::DesktopPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DesktopPath)),
                        false);
}

void Core::initialize()
{
    registerFileSystem();
    registerAllService();
    GlobalPrivate::screens = ScreenService::instance()->allScreen(qApp->platformName());
    for (auto val : GlobalPrivate::screens) {
        //注册到服务自动管理
        BackgroundService::regClass<BackgroundDefault>(val->name());
        BackgroundService::create(val->name());
    }
    GlobalPrivate::backgrounds = BackgroundService::instance()->allBackground();
    //获取所有的背景
}

bool Core::start()
{
    // qInfo() << PlatformTypes::XCB << ScreenService::instance()->allScreen(PlatformTypes::XCB);
    // qInfo() << PlatformTypes::WAYLAND << ScreenService::instance()->allScreen(PlatformTypes::WAYLAND);

    for (auto val: GlobalPrivate::backgrounds) { //此处可线程并发设置图片
        val->setDisplay("/usr/share/wallpapers/deepin/abc-123.jpg"); //这里需要从WallpaperService中获取壁纸，先做测试
        //        val->setWindowFlags(Qt::FramelessWindowHint);
        val->setAttribute(Qt::WA_TranslucentBackground,true);
        val->setWindowOpacity(1);
        val->show();
    }
    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return Sync;
}
