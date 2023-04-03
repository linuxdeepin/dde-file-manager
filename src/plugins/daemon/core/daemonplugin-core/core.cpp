// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

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
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
    // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
}

bool Core::start()
{
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
