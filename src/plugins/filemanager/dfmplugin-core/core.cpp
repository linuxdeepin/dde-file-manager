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
#include "corelog.h"

#include "services/filemanager/window/windowservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/file/local/localfilemenu.h"
#include "dfm-base/widgets/dfmfileview/fileview.h"
#include "dfm-base/widgets/filemanagerwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE

namespace GlobalPrivate {
const int kMinimumWindowWidth = 760;
const int kMinimumWindowHeight = 420;
const int kDefaultWindowWidth = 1100;
const int kDefaultWindowHeight = 700;
static Application *kDFMApp = nullptr;
}   // namespace GlobalPrivate

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

    // 注册路由
    UrlRoute::regScheme(SchemeTypes::kFile, "/");
    // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kFile);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kFile);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kFile);
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
        QString error;
        FileManagerWindow *window = windowService->showWindow(defaultUrl, true, &error);
        if (window) {
            window->resize(GlobalPrivate::kDefaultWindowWidth, GlobalPrivate::kDefaultWindowHeight);
            window->setMinimumSize(GlobalPrivate::kMinimumWindowWidth,
                                   GlobalPrivate::kMinimumWindowHeight);
        } else {
            qWarning() << "Cannot show window: " << error << " Url is:" << defaultUrl;
        }
    }

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return kSync;
}
