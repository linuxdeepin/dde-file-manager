/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "core.h"
#include "corelog.h"
#include "eventcaller.h"
#include "corebrowseview.h"

#include "windowservice.h"
#include "services/common/menu/menuservice.h"

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

#include "dfm-framework/framework.h"
#include "dfm-framework/listener/listener.h"
#include "dfm-framework/lifecycle/plugin.h"
#include "dfm-framework/service/pluginservice.h"

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

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE

namespace GlobalPrivate {
    const int DEFAULT_WINDOW_WIDTH = 760;
    const int DEFAULT_WINDOW_HEIGHT = 420;
    static Application* dfmApp = nullptr;
}

//DFM_USE_NAMESPACE

void initSidebar(SideBar* sidebar)
{
    if (!sidebar) return ;

    QUrl homeUrl = UrlRoute::pathToReal(QDir::home().path());
    QUrl desktopUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::DesktopPath));
    QUrl videosUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::VideosPath));
    QUrl musicUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::MusicPath));
    QUrl picturesUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::PicturesPath));
    QUrl documentsUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::DocumentsPath));
    QUrl downloadsUrl = UrlRoute::pathToReal(StandardPaths::location(StandardPaths::DownloadsPath));

    QIcon homeIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::HomePath));
    QIcon desktopIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DesktopPath));
    QIcon videosIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::VideosPath));
    QIcon musicIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::MusicPath));
    QIcon picturesIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::PicturesPath));
    QIcon documentsIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DocumentsPath));
    QIcon downloadsIcon = QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DownloadsPath));

    auto homeItem = new SideBarItem(homeIcon,QObject::tr("Home"),"core",homeUrl);
    homeItem->setFlags(homeItem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto desktopitem = new SideBarItem(desktopIcon,QObject::tr("Desktop"),"core",desktopUrl);
    desktopitem->setFlags(desktopitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto videoitem = new SideBarItem(videosIcon,QObject::tr("Video"),"core",videosUrl);
    videoitem->setFlags(videoitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto musicitem = new SideBarItem(musicIcon,QObject::tr("Music"),"core",musicUrl);
    musicitem->setFlags(musicitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto picturesitem = new SideBarItem(picturesIcon, QObject::tr("Pictures"), "core", picturesUrl);
    picturesitem->setFlags(picturesitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto documentsitem = new SideBarItem(documentsIcon, QObject::tr("Documents"), "core", documentsUrl);
    documentsitem->setFlags(documentsitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto downloadsitem = new SideBarItem(downloadsIcon, QObject::tr("Downloads"), "core", downloadsUrl);
    downloadsitem->setFlags(downloadsitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    qInfo() <<  sidebar->addItem(homeItem);
    qInfo() <<  sidebar->addItem(desktopitem);
    qInfo() <<  sidebar->addItem(videoitem);
    qInfo() <<  sidebar->addItem(musicitem);
    qInfo() <<  sidebar->addItem(picturesitem);
    qInfo() <<  sidebar->addItem(documentsitem);
    qInfo() <<  sidebar->addItem(downloadsitem);

    sidebar->setMinimumWidth(120);
    sidebar->setMaximumWidth(200);
}

static void regStandardPathClass()
{
    UrlRoute::regScheme(SchemeTypes::HOME,
                        QDir::home().path(),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::HomePath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::DESKTOP,
                        StandardPaths::location(StandardPaths::DesktopPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DesktopPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::VIDEOS,
                        StandardPaths::location(StandardPaths::VideosPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::VideosPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::MUSIC,
                        StandardPaths::location(StandardPaths::MusicPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::MusicPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::PICTURES,
                        StandardPaths::location(StandardPaths::PicturesPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::PicturesPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::DOCUMENTS,
                        StandardPaths::location(StandardPaths::DocumentsPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DocumentsPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::DOWNLOADS,
                        StandardPaths::location(StandardPaths::DownloadsPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DownloadsPath)),
                        false);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::HOME);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::HOME);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::HOME);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::HOME);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::HOME);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::DESKTOP);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::DESKTOP);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::DESKTOP);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::DESKTOP);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::DESKTOP);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::VIDEOS);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::VIDEOS);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::VIDEOS);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::VIDEOS);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::VIDEOS);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::MUSIC);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::MUSIC);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::MUSIC);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::MUSIC);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::MUSIC);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::PICTURES);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::PICTURES);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::PICTURES);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::PICTURES);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::PICTURES);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::DOCUMENTS);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::DOCUMENTS);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::DOCUMENTS);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::DOCUMENTS);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::DOCUMENTS);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::DOWNLOADS);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::DOWNLOADS);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::DOWNLOADS);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::DOWNLOADS);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::DOWNLOADS);
}

void Core::initialize()
{
    dpfCritical() << __PRETTY_FUNCTION__;
    qInfo() << Q_FUNC_INFO;

    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(WindowService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    //注册路由
    UrlRoute::regScheme(SchemeTypes::FILE,"/");
    //注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::FILE);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::FILE);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::FILE);
    BrowseWidgetFactory::regClass<BrowseView>(SchemeTypes::FILE);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::FILE);

    regStandardPathClass();
}

bool Core::start()
{
    GlobalPrivate::dfmApp = new Application;
    dpfCritical() << __PRETTY_FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    qInfo() << "import service list" <<  ctx.services();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (!windowService) {
        qCCritical(CorePlugin) << "Failed, init window \"windowService\" is empty";
        return false;
    }

    if (windowService) {
        QUrl localRootUrl = QUrl::fromLocalFile("/");
        QUrl defaultUrl = UrlRoute::pathToReal(QDir::home().path());
        BrowseWindow *newWindow = windowService->newWindow();

        if (newWindow){
            int winIdx = windowService->windowList.indexOf(newWindow);
            // 绑定当前插件初始化完毕进行的相关操作。
            QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                             this, [winIdx](){
                // 发送打开的新窗口的事件
                EventCaller::sendOpenNewWindowEvent(winIdx);
            });

            // 初始化sidebar
            initSidebar(newWindow->sidebar());;
            newWindow->show();
            newWindow->setMinimumSize(GlobalPrivate::DEFAULT_WINDOW_WIDTH,
                                      GlobalPrivate::DEFAULT_WINDOW_HEIGHT);

            // 綁定sidebaritem的點擊邏輯
            QObject::connect(newWindow->sidebar(), &SideBar::clickedItemUrl,
                             newWindow, [windowService, newWindow](const QUrl &url)
            {
                bool result = windowService->setWindowRootUrl(newWindow, url);
                if (!result)
                    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
            });
        }
        // 设置主目录
        windowService->setWindowRootUrl(newWindow, defaultUrl);
    }

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return Sync;
}
