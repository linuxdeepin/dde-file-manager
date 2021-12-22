/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#include "services/filemanager/windows/windowsservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/file/local/localfilemenu.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE

namespace GlobalPrivate {
static Application *kDFMApp = nullptr;
}   // namespace GlobalPrivate

static void regStandardPathClass()
{
    UrlRoute::regScheme(SchemeTypes::kHome,
                        QDir::home().path(),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kHomePath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kDesktop,
                        StandardPaths::location(StandardPaths::kDesktopPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kDesktopPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kVideos,
                        StandardPaths::location(StandardPaths::kVideosPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kVideosPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kMusic,
                        StandardPaths::location(StandardPaths::kMusicPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kMusicPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kPictures,
                        StandardPaths::location(StandardPaths::kPicturesPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kPicturesPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kDocuments,
                        StandardPaths::location(StandardPaths::kDocumentsPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kDocumentsPath)),
                        false);

    UrlRoute::regScheme(SchemeTypes::kDownloads,
                        StandardPaths::location(StandardPaths::kDownloadsPath),
                        QIcon::fromTheme(StandardPaths::iconName(StandardPaths::kDownloadsPath)),
                        false);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kHome);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kHome);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kHome);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kHome);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kDesktop);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kDesktop);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kDesktop);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kDesktop);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kVideos);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kVideos);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kVideos);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kVideos);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kMusic);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kMusic);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kMusic);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kMusic);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kPictures);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kPictures);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kPictures);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kPictures);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kDocuments);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kDocuments);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kDocuments);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kDocuments);

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kDownloads);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kDownloads);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kDownloads);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kDownloads);
}

void Core::initialize()
{
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(WindowsService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    // 注册路由
    UrlRoute::regScheme(SchemeTypes::kFile, "/");
    // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kFile);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kFile);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kFile);
    regStandardPathClass();
}

bool Core::start()
{
    GlobalPrivate::kDFMApp = new Application;
    qDebug() << __PRETTY_FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    qInfo() << "import service list" << ctx.services();
    WindowsService *windowService = ctx.service<WindowsService>(WindowsService::name());

    if (!windowService) {
        qCritical() << "Failed, init window \"windowService\" is empty";
        return false;
    }

    if (windowService) {
        // TODO(zhangs): default url should read it from config file
        QUrl defaultUrl = UrlRoute::pathToReal(QDir::home().path());
        QString error;
        FileManagerWindow *window = windowService->showWindow(defaultUrl, true, &error);
        if (!window)
            qWarning() << "Cannot show window: " << error << " Url is:" << defaultUrl;
    }

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return kSync;
}
