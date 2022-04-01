/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "core.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/base/device/devicecontroller.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <unistd.h>

DAEMONPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static constexpr char kDaemonServicePath[] { "com.deepin.filemanager.daemon" };
static constexpr char kEnvNameOfDaemonRegistered[] { "DAEMON_SERVICE_REGISTERED" };

void Core::initialize()
{
    // 注册路由
    UrlRoute::regScheme(Global::kFile, "/");
    // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<LocalFileInfo>(Global::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::kFile);
}

bool Core::start()
{
    DeviceController::instance()->startMonitor();

    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.interface()->isServiceRegistered(kDaemonServicePath)) {
        qInfo() << connection.registerService(kDaemonServicePath) << "register" << kDaemonServicePath << "success";
        qputenv(kEnvNameOfDaemonRegistered, "TRUE");
    } else {
        qWarning() << connection.registerService(kDaemonServicePath) << "register" << kDaemonServicePath << "failed";
        qputenv(kEnvNameOfDaemonRegistered, "FALSE");
    }

    return true;
}
