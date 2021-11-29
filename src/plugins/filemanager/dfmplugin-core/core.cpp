/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE

namespace GlobalPrivate {
const int kDefaultWindowWidth = 760;
const int kDefaultWindowHeight = 420;
static Application *kDFMApp = nullptr;
}

//DFM_USE_NAMESPACE

void initSidebar(SideBar *sidebar)
{
    if (!sidebar) return;

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

    sidebar->setMinimumWidth(120);
    sidebar->setMaximumWidth(200);
}

static void regStandardPathClass()
{
    UrlRoute::regScheme(SchemeTypes::kHome,
                        QDir::home().path(),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::HomePath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kDesktop,
                        StandardPaths::location(StandardPaths::DesktopPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DesktopPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kVideos,
                        StandardPaths::location(StandardPaths::VideosPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::VideosPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kMusic,
                        StandardPaths::location(StandardPaths::MusicPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::MusicPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kPictures,
                        StandardPaths::location(StandardPaths::PicturesPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::PicturesPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kDocuments,
                        StandardPaths::location(StandardPaths::DocumentsPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DocumentsPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kDownloads,
                        StandardPaths::location(StandardPaths::DownloadsPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::DownloadsPath)),
                        false);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kHome);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kHome);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kHome);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::kHome);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kHome);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kDesktop);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kDesktop);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kDesktop);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::kDesktop);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kDesktop);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kVideos);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kVideos);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kVideos);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::kVideos);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kVideos);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kMusic);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kMusic);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kMusic);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::kMusic);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kMusic);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kPictures);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kPictures);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kPictures);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::kPictures);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kPictures);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kDocuments);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kDocuments);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kDocuments);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::kDocuments);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kDocuments);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kDownloads);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kDownloads);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kDownloads);
    BrowseWidgetFactory::regClass<CoreBrowseView>(SchemeTypes::kDownloads);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kDownloads);
}

void Core::initialize()
{
    dpfDebug() << __PRETTY_FUNCTION__;
    qInfo() << Q_FUNC_INFO;

    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(WindowService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    //注册路由
    UrlRoute::regScheme(SchemeTypes::kFile, "/");
    //注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kFile);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kFile);
    BrowseWidgetFactory::regClass<BrowseView>(SchemeTypes::kFile);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kFile);

    regStandardPathClass();
}

bool Core::start()
{
    GlobalPrivate::kDFMApp = new Application;
    dpfDebug() << __PRETTY_FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    qInfo() << "import service list" << ctx.services();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (!windowService) {
        qCCritical(CorePlugin) << "Failed, init window \"windowService\" is empty";
        return false;
    }

    if (windowService) {
        QUrl localRootUrl = QUrl::fromLocalFile("/");
        QUrl defaultUrl = UrlRoute::pathToReal(QDir::home().path());
        BrowseWindow *newWindow = windowService->newWindow();

        if (newWindow) {
            quint64 winIdx = newWindow->internalWinId();
            // 绑定当前插件初始化完毕进行的相关操作。
            QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                             this, [winIdx]() {
                                 // 发送打开的新窗口的事件
                                 EventCaller::sendOpenNewWindowEvent(winIdx);
                             });

            // 初始化sidebar
            initSidebar(newWindow->sidebar());
            ;
            newWindow->show();
            newWindow->setMinimumSize(GlobalPrivate::kDefaultWindowWidth,
                                      GlobalPrivate::kDefaultWindowHeight);

            // 綁定sidebaritem的點擊邏輯
            QObject::connect(newWindow->sidebar(), &SideBar::clickedItemUrl,
                             newWindow, [windowService, newWindow](const QUrl &url) {
                                 bool result = windowService->setWindowRootUrl(newWindow, url);
                                 if (!result)
                                     QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
                             });

            // 绑定Sidebar右键菜单信号
            QObject::connect(newWindow->sidebar(), &SideBar::customContextMenu,
                             newWindow, [=](const QUrl &url, const QPoint &pos) {
                                 // 发送侧边栏右键菜单事件
                                 EventCaller::sendSideBarContextMenuEvent(url, pos);
                             });

            // 绑定搜索逻辑
            QObject::connect(newWindow->addresssBar(), &AddressBar::editingFinishedSearch,
                             newWindow, [newWindow, winIdx](const QString &keyword) {
                                 // 发送搜索事件
                                 EventCaller::sendSearchEvent(newWindow->rootUrl(), keyword, winIdx);
                             });
        }
        // 设置主目录
        windowService->setWindowRootUrl(newWindow, defaultUrl);
    }

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return kSync;
}
