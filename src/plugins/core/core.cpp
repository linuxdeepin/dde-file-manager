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
#include "eventcalled.h"

#include "applicationservice/application.h"
#include "windowservice/windowservice.h"
#include "windowservice/browseview.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/abstractfilewatcher.h"
#include "dfm-base/localfile/localfileinfo.h"
#include "dfm-base/localfile/localdiriterator.h"
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"
#include "dfm-base/widgets/dfmfileview/fileview.h"

#include "dfm-framework/listener/listener.h"
#include "dfm-framework/lifecycle/plugin.h"

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

DSB_FM_USE_NAMESPACE

namespace GlobalPrivate {
    const int DEFAULT_WINDOW_WIDTH = 760;
    const int DEFAULT_WINDOW_HEIGHT = 420;
    static Application* dfmApp = nullptr;
}

//DFM_USE_NAMESPACE

void initSidebar(SideBar* sidebar)
{
    if (!sidebar) return ;

    QUrl homeUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::HomePath));
    QUrl desktopUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::DesktopPath));
    QUrl videosUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::VideosPath));
    QUrl musicUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::MusicPath));
    QUrl picturesUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::PicturesPath));
    QUrl documentsUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::DocumentsPath));
    QUrl downloadsUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::DownloadsPath));

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
    UrlRoute::schemeMapRoot("home",
                            StandardPaths::location(StandardPaths::HomePath),
                            QIcon::fromTheme(StandardPaths::iconName(StandardPaths::HomePath)),
                            false);

    UrlRoute::schemeMapRoot("desktop",
                            StandardPaths::location(StandardPaths::DesktopPath),
                            QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DesktopPath)),
                            false);

    UrlRoute::schemeMapRoot("videos",
                            StandardPaths::location(StandardPaths::VideosPath),
                            QIcon::fromTheme(StandardPaths::iconName(StandardPaths::VideosPath)),
                            false);

    UrlRoute::schemeMapRoot("music",
                            StandardPaths::location(StandardPaths::MusicPath),
                            QIcon::fromTheme(StandardPaths::iconName(StandardPaths::MusicPath)),
                            false);

    UrlRoute::schemeMapRoot("pictures",
                            StandardPaths::location(StandardPaths::PicturesPath),
                            QIcon::fromTheme(StandardPaths::iconName(StandardPaths::PicturesPath)),
                            false);

    UrlRoute::schemeMapRoot("documents",
                            StandardPaths::location(StandardPaths::DocumentsPath),
                            QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DocumentsPath)),
                            false);

    UrlRoute::schemeMapRoot("downloads",
                            StandardPaths::location(StandardPaths::DownloadsPath),
                            QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DownloadsPath)),
                            false);

    InfoFactory::instance().regClass<LocalFileInfo>("home");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("home");
    WacherFactory::instance().regClass<AbstractFileWatcher>("home");
    BrowseWidgetFactory::instance().regClass<BrowseView>("home");

    InfoFactory::instance().regClass<LocalFileInfo>("desktop");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("desktop");
    WacherFactory::instance().regClass<AbstractFileWatcher>("desktop");
    BrowseWidgetFactory::instance().regClass<BrowseView>("desktop");

    InfoFactory::instance().regClass<LocalFileInfo>("videos");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("videos");
    WacherFactory::instance().regClass<AbstractFileWatcher>("videos");
    BrowseWidgetFactory::instance().regClass<BrowseView>("videos");

    InfoFactory::instance().regClass<LocalFileInfo>("music");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("music");
    WacherFactory::instance().regClass<AbstractFileWatcher>("music");
    BrowseWidgetFactory::instance().regClass<BrowseView>("music");

    InfoFactory::instance().regClass<LocalFileInfo>("pictures");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("pictures");
    WacherFactory::instance().regClass<AbstractFileWatcher>("pictures");
    BrowseWidgetFactory::instance().regClass<BrowseView>("pictures");

    InfoFactory::instance().regClass<LocalFileInfo>("documents");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("documents");
    WacherFactory::instance().regClass<AbstractFileWatcher>("documents");
    BrowseWidgetFactory::instance().regClass<BrowseView>("documents");

    InfoFactory::instance().regClass<LocalFileInfo>("downloads");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("downloads");
    WacherFactory::instance().regClass<AbstractFileWatcher>("downloads");
    BrowseWidgetFactory::instance().regClass<BrowseView>("downloads");
}

void Core::initialize()
{
    dpfCritical() << __PRETTY_FUNCTION__;
    qInfo() << Q_FUNC_INFO;

    IMPORT_SERVICE(WindowService);
    //    IMPORT_SERVICE(DFMOldPreviewService);

    //注册路由
    UrlRoute::schemeMapRoot("file","/");
    //注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::instance().regClass<LocalFileInfo>("file");
    DirIteratorFactory::instance().regClass<LocalDirIterator>("file");
    WacherFactory::instance().regClass<AbstractFileWatcher>("file");
    BrowseWidgetFactory::instance().regClass<BrowseView>("file");

    regStandardPathClass();
}

bool Core::start(QSharedPointer<dpf::PluginContext> context)
{
    GlobalPrivate::dfmApp = new Application;
    dpfCritical() << __PRETTY_FUNCTION__;
    qInfo() << "import service list" << context->services();
    //    auto previewService = context->service<DFMOldPreviewService>("DFMOldPreviewService");
    //    if (previewService) {
    //        auto viewInterfaces = previewService->getViewInterfaces();
    //        qInfo() << "viewInterfaces.size()" << viewInterfaces.size();
    //        auto previewInterfaces = previewService->getPreviewInterfaces();
    //        qInfo() << "previewInterfaces.size()" << previewInterfaces.size();
    //        auto expandInfoInterfaces = previewService->getExpandInfoInterfaces();
    //        qInfo() << "expandInfoInterfaces.size()" << expandInfoInterfaces.size();
    //        auto dfmfilePreviewInterface = previewService->getDFMFilePreviewInterface();
    //        qInfo() << "dfmfilePreviewInterface.size()" << dfmfilePreviewInterface.size();
    //    }

    auto windowService = context->service<WindowService>("WindowService");

    if (!windowService) {
        qCCritical(CorePlugin) << "Failed, init window \"windowService\" is empty";
        return false;
    }

    if (windowService) {
        QUrl defaultUrl = UrlRoute::pathToUrl(StandardPaths::location(StandardPaths::HomePath));
        BrowseWindow* newWindow = windowService->newWindow();

        if (newWindow){
            int winIdx = windowService->windowList.indexOf(newWindow);

            // 绑定当前插件初始化完毕进行的相关操作。
            QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                             this, [winIdx](){
                // 发送打开的新窗口的事件
                EventCalled::sendOpenNewWindowEvent(winIdx);
            });

            // 初始化sidebar
            initSidebar(newWindow->sidebar());;
            newWindow->show();
            newWindow->setMinimumSize(GlobalPrivate::DEFAULT_WINDOW_WIDTH,
                                      GlobalPrivate::DEFAULT_WINDOW_HEIGHT);

            //綁定sidebaritem的點擊邏輯
            QObject::connect(newWindow->sidebar(), &SideBar::activatedItemUrl,
                             newWindow, [windowService, newWindow](const QUrl &url)
            {
                windowService->setWindowRootUrl(newWindow, url);
            });
        }

        // 设置默认file
        windowService->setWindowRootUrl(newWindow, defaultUrl);
    }

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop() {

    EXPORT_SERVICE(WindowService);
    //    EXPORT_SERVICE(DFMOldPreviewService);

    return Synch;
}
