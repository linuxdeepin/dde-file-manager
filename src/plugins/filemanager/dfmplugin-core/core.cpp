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
#include "events/coreeventreceiver.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/command/commandservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/dfm_global_defines.h"

#include <dfm-framework/dpf.h>
#include <dfm-mount/ddevicemanager.h>

#include <QTimer>
#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
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
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<LocalFileInfo>(Global::Scheme::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
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

    // mount business
    if (!DevProxyMng->connectToService()) {
        qCritical() << "device manager cannot connect to service!";
        DevMngIns->startMonitor();
        DevMngIns->startPollingDeviceUsage();
    }

    // show first window when all plugin initialized
    connect(dpfListener, &dpf::Listener::pluginsInitialized, this, &Core::onAllPluginsInitialized);

    connect(dpfListener, &dpf::Listener::pluginsStarted, this, &Core::onAllPluginsStarted);

    // the object must be initialized in main thread, otherwise the GVolumeMonitor do not have an event loop.
    static std::once_flag flg;
    std::call_once(flg, [this] {
        QTimer::singleShot(500, this, [] { DFMMOUNT::DDeviceManager::instance(); });
    });

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return kSync;
}

void Core::onAllPluginsInitialized()
{
    // subscribe events
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl,
                                   CoreEventReceiver::instance(), &CoreEventReceiver::handleChangeUrl);
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewWindow,
                                   CoreEventReceiver::instance(), &CoreEventReceiver::handleOpenWindow);
}

void Core::onAllPluginsStarted()
{
    // dde-select-dialog also uses the core plugin, don't start filemanger window
    QString &&curAppName { qApp->applicationName() };
    if (curAppName == "dde-file-manager")
        commandServIns->processCommand();
    else
        qInfo() << "Current app name is: " << curAppName << " Don't show filemanger window";
}
