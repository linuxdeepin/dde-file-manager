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
//new lib
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"

#include "dfm-base/widgets/dfmfileview/fileview.h"
//#include "widgets/dfmfileview/dfilesystemmodel.h"

#include "dfm-base/base/dfmstandardpaths.h"
#include "dfm-base/base/dfmapplication.h"
#include "dfm-base/base/dfmschemefactory.h"

#include "dfm-base/localfile/dfmlocalfileinfo.h"
#include "dfm-base/localfile/dfmlocaldiriterator.h"
#include "dfm-base/localfile/dfmlocalfiledevice.h"
#include "dfm-base/base/dabstractfilewatcher.h"

//old lib
//#include "interfaces/dfmapplication.h"
//#include "views/dfmsidebar.h"
//#include "views/dfileview.h"
//#include "views/dfmsidebarview.h"
//#include "models/dfmsidebarmodel.h"
//#include "interfaces/dfmsidebaritem.h"
//#include "interfaces/dfilesystemmodel.h"
//#include "views/dfilemanagerwindow.h"
//#include "views/dtoolbar.h"

#include "core.h"
#include "corelog.h"
#include "dfm-framework/lifecycle/plugin.h"

////services
//#include "previewservice/dfmoldpreviewservice.h"
////old plugin base
//#include "previewservice/oldpluginbase/dfmfilepreviewplugin.h"
//#include "previewservice/oldpluginbase/dfmfilepreview.h"

#include "windowservice/windowservice.h"
#include "windowservice/browseview.h"

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

DSB_DM_USE_NAMESPACE

namespace GlobalPrivate {
    const int DEFAULT_WINDOW_WIDTH = 760;
    const int DEFAULT_WINDOW_HEIGHT = 420;
    static DFMApplication* dfmApp = nullptr;
}

//DFM_USE_NAMESPACE

void initSidebar(DFMSideBar* sidebar)
{
    if (!sidebar) return ;

    QUrl homeUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::HomePath));
    QUrl desktopUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::DesktopPath));
    QUrl videosUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::VideosPath));
    QUrl musicUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::MusicPath));
    QUrl picturesUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::PicturesPath));
    QUrl documentsUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::DocumentsPath));
    QUrl downloadsUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::DownloadsPath));

    QIcon homeIcon = QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::HomePath));
    QIcon desktopIcon = QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::DesktopPath));
    QIcon videosIcon = QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::VideosPath));
    QIcon musicIcon = QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::MusicPath));
    QIcon picturesIcon = QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::PicturesPath));
    QIcon documentsIcon = QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::DocumentsPath));
    QIcon downloadsIcon = QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::DownloadsPath));

    auto homeItem = new DFMSideBarItem(homeIcon,QObject::tr("Home"),"core",homeUrl);
    homeItem->setFlags(homeItem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto desktopitem = new DFMSideBarItem(desktopIcon,QObject::tr("Desktop"),"core",desktopUrl);
    desktopitem->setFlags(desktopitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto videoitem = new DFMSideBarItem(videosIcon,QObject::tr("Video"),"core",videosUrl);
    videoitem->setFlags(videoitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto musicitem = new DFMSideBarItem(musicIcon,QObject::tr("Music"),"core",musicUrl);
    musicitem->setFlags(musicitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto picturesitem = new DFMSideBarItem(picturesIcon, QObject::tr("Pictures"), "core", picturesUrl);
    picturesitem->setFlags(picturesitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto documentsitem = new DFMSideBarItem(documentsIcon, QObject::tr("Documents"), "core", documentsUrl);
    documentsitem->setFlags(documentsitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    auto downloadsitem = new DFMSideBarItem(downloadsIcon, QObject::tr("Downloads"), "core", downloadsUrl);
    downloadsitem->setFlags(downloadsitem->flags()&~(Qt::ItemIsEditable|Qt::ItemIsDragEnabled));

    sidebar->addItem(homeItem,"default");
    sidebar->addItem(desktopitem,"default");
    sidebar->addItem(videoitem,"default");
    sidebar->addItem(musicitem,"default");
    sidebar->addItem(picturesitem,"default");
    sidebar->addItem(documentsitem,"default");
    sidebar->addItem(downloadsitem,"default");

    sidebar->setMinimumWidth(120);
    sidebar->setMaximumWidth(200);
}

static void regStandardPathClass()
{
    DFMUrlRoute::schemeMapRoot("home",
                               DFMStandardPaths::location(DFMStandardPaths::HomePath),
                               QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::HomePath)),
                               false);

    DFMUrlRoute::schemeMapRoot("desktop",
                               DFMStandardPaths::location(DFMStandardPaths::DesktopPath),
                               QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::DesktopPath)),
                               false);

    DFMUrlRoute::schemeMapRoot("videos",
                               DFMStandardPaths::location(DFMStandardPaths::VideosPath),
                               QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::VideosPath)),
                               false);

    DFMUrlRoute::schemeMapRoot("music",
                               DFMStandardPaths::location(DFMStandardPaths::MusicPath),
                               QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::MusicPath)),
                               false);

    DFMUrlRoute::schemeMapRoot("pictures",
                               DFMStandardPaths::location(DFMStandardPaths::PicturesPath),
                               QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::PicturesPath)),
                               false);

    DFMUrlRoute::schemeMapRoot("documents",
                               DFMStandardPaths::location(DFMStandardPaths::DocumentsPath),
                               QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::DocumentsPath)),
                               false);

    DFMUrlRoute::schemeMapRoot("downloads",
                               DFMStandardPaths::location(DFMStandardPaths::DownloadsPath),
                               QIcon::fromTheme(DFMStandardPaths::iconName(DFMStandardPaths::DownloadsPath)),
                               false);

    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("home");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("home");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("home");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("home");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("home");

    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("desktop");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("desktop");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("desktop");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("desktop");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("desktop");

    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("videos");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("videos");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("videos");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("videos");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("videos");

    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("music");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("music");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("music");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("music");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("music");

    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("pictures");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("pictures");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("pictures");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("pictures");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("pictures");

    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("documents");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("documents");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("documents");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("documents");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("documents");

    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("downloads");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("downloads");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("downloads");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("downloads");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("downloads");
}

void Core::initialize()
{
    dpfCritical() << __PRETTY_FUNCTION__;
    qInfo() << Q_FUNC_INFO;

    IMPORT_SERVICE(WindowService);
    //    IMPORT_SERVICE(DFMOldPreviewService);

    //注册路由
    DFMUrlRoute::schemeMapRoot("file","/");
    //注册Scheme为"file"的扩展的文件信息 本地默认文件的
    DFMInfoFactory::instance().regClass<DFMLocalFileInfo>("file");
    DFMFileDeviceFactory::instance().regClass<DFMLocalFileDevice>("file");
    DFMDirIteratorFactory::instance().regClass<DFMLocalDirIterator>("file");
    DFMWacherFactory::instance().regClass<DAbstractFileWatcher>("file");
    DFMBrowseWidgetFactory::instance().regClass<DFMBrowseView>("file");

    regStandardPathClass();
}

bool Core::start(QSharedPointer<dpf::PluginContext> context)
{
    GlobalPrivate::dfmApp = new DFMApplication;
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
        QUrl defaultUrl = DFMUrlRoute::pathToUrl(DFMStandardPaths::location(DFMStandardPaths::HomePath));
        DFMBrowseWindow* newWindow = windowService->newWindow();

        if (newWindow){
            int winIdx = windowService->m_windowlist.indexOf(newWindow);
            //send new window event to bundles

            //初始化sidebar
            initSidebar(newWindow->sidebar());;
            newWindow->show();
            newWindow->setMinimumSize(GlobalPrivate::DEFAULT_WINDOW_WIDTH,
                                      GlobalPrivate::DEFAULT_WINDOW_HEIGHT);

            //綁定sidebaritem的點擊邏輯
            QObject::connect(newWindow->sidebar(), &DFMSideBar::activatedItemUrl,
                             newWindow, [=](const QUrl &url)
            {
                windowService->setWindowRootUrl(newWindow, url);
            });
        }

        //设置默认file
        windowService->setWindowRootUrl(newWindow, defaultUrl);
    }

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop() {

    EXPORT_SERVICE(WindowService);
    //    EXPORT_SERVICE(DFMOldPreviewService);

    return Synch;
}
