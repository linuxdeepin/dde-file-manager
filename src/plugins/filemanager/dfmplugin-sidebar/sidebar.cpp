/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "sidebar.h"
#include "sidebarwidget.h"
#include "sidebaritem.h"

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/standardpaths.h"

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static void initSideBar(SideBarWidget *sidebar)
{
    Q_ASSERT_X(sidebar, "SideBar", "SideBar is NULL");

    QUrl &&homeUrl = UrlRoute::pathToReal(QDir::home().path());
    QUrl &&desktopUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::kDesktopPath));
    QUrl &&videosUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::kVideosPath));
    QUrl &&musicUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::kMusicPath));
    QUrl &&picturesUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::kPicturesPath));
    QUrl &&documentsUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::kDocumentsPath));
    QUrl &&downloadsUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::kDownloadsPath));

    // TODO(zhangs): follow icons is error, fix it
    QIcon &&homeIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kHomePath));
    QIcon &&desktopIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kDesktopPath));
    QIcon &&videosIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kVideosPath));
    QIcon &&musicIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kMusicPath));
    QIcon &&picturesIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kPicturesPath));
    QIcon &&documentsIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kDocumentsPath));
    QIcon &&downloadsIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kDownloadsPath));

    auto homeItem = new SideBarItem(homeIcon, QObject::tr("Home"), "core", homeUrl);
    homeItem->setFlags(homeItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

    auto desktopitem = new SideBarItem(desktopIcon, QObject::tr("Desktop"), "core", desktopUrl);
    desktopitem->setFlags(desktopitem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

    auto videoitem = new SideBarItem(videosIcon, QObject::tr("Video"), "core", videosUrl);
    videoitem->setFlags(videoitem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

    auto musicitem = new SideBarItem(musicIcon, QObject::tr("Music"), "core", musicUrl);
    musicitem->setFlags(musicitem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

    auto picturesitem = new SideBarItem(picturesIcon, QObject::tr("Pictures"), "core", picturesUrl);
    picturesitem->setFlags(picturesitem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

    auto documentsitem = new SideBarItem(documentsIcon, QObject::tr("Documents"), "core", documentsUrl);
    documentsitem->setFlags(documentsitem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

    auto downloadsitem = new SideBarItem(downloadsIcon, QObject::tr("Downloads"), "core", downloadsUrl);
    downloadsitem->setFlags(downloadsitem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled));

    sidebar->addItem(homeItem);
    sidebar->addItem(desktopitem);
    sidebar->addItem(videoitem);
    sidebar->addItem(musicitem);
    sidebar->addItem(picturesitem);
    sidebar->addItem(documentsitem);
    sidebar->addItem(downloadsitem);
}

void SideBar::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowsService *windowService = ctx.service<WindowsService>(WindowsService::name());
    Q_ASSERT_X(!windowService->windowIdList().isEmpty(), "SideBar", "Cannot acquire any window");
    // get first window
    quint64 id = windowService->windowIdList().first();
    auto window = windowService->findWindowById(id);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    auto sidebar = new SideBarWidget;
    initSideBar(sidebar);
    window->setSideBar(sidebar);
}

bool SideBar::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag SideBar::stop()
{
    return kSync;
}
