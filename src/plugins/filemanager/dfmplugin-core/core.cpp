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
#include "coreeventreceiver.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/common/menu/menuservice.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/file/local/localfilemenu.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>

DPCORE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE

namespace GlobalPrivate {
static WindowsService *windowService { nullptr };
static Application *kDFMApp { nullptr };
}   // namespace GlobalPrivate

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
    // 初始化剪切板
    ClipBoard::instance();
}

bool Core::start()
{
    qDebug() << __PRETTY_FUNCTION__;
    GlobalPrivate::kDFMApp = new Application;   // must create it
    auto &ctx = dpfInstance.serviceContext();
    qInfo() << "import service list" << ctx.services();
    GlobalPrivate::windowService = ctx.service<WindowsService>(WindowsService::name());
    if (!GlobalPrivate::windowService) {
        qCritical() << "Failed, init window \"windowService\" is empty";
        abort();
    }

    // show first window when all plugin initialized
    connect(&dpfInstance.listener(), &dpf::Listener::pluginsInitialized, this, &Core::onAllPluginsInitialized);

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return kSync;
}

void Core::onAllPluginsInitialized()
{
    QString error;
    FileManagerWindow *window = GlobalPrivate::windowService->showWindow(QUrl(), true, &error);
    if (!window) {
        qWarning() << "Cannot show window: " << error;
        abort();
    }

    // subscribe events
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kChangeCurrentUrl,
                                            CoreEventReceiver::instance(), &CoreEventReceiver::handleChangeUrl);
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenNewWindow,
                                            CoreEventReceiver::instance(), &CoreEventReceiver::handleOpenWindow);
}
