// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"

#include "devicemanageradaptor.h"
#include "operationsstackmanageradaptor.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <QDBusConnection>

DAEMONPCORE_BEGIN_NAMESPACE
DFM_LOG_REISGER_CATEGORY(DAEMONPCORE_NAMESPACE)

DFMBASE_USE_NAMESPACE

static constexpr char kDaemonName[] { "org.deepin.Filemanager.Daemon" };
static constexpr char kDeviceManagerObjPath[] { "/org/deepin/Filemanager/Daemon/DeviceManager" };
static constexpr char kOperationsStackManagerObjPath[] { "/org/deepin/Filemanager/Daemon/OperationsStackManager" };

void Core::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
}

bool Core::start()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        fmWarning("Cannot connect to the D-Bus session bus.\n"
                  "Please check your system settings and try again.\n");
        return false;
    }

    initServiceDBusInterfaces(&connection);

    // FIXME(XUST) server should directly use DeviceManager instance,
    // the DBus methods are provided by server, the DeviceManager is intanced in
    // DeviceManagerDBus class, so this code seems useless.
    // but now it doesn't bring me any issue, left now and remove it later.
    if (!DevProxyMng->initService()) {
        fmCritical() << "device manager cannot connect to server!";
        DevMngIns->startMonitor();
    }

    bool connected = QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          "PrepareForShutdown",
                                                          this,
                                                          SLOT(exitOnShutdown(bool)));
    fmDebug() << "login1::PrepareForShutdown connected:" << connected;

    return true;
}

void Core::initServiceDBusInterfaces(QDBusConnection *connection)
{
    // TODO(zhangs): move to thread!
    static std::once_flag flag;
    std::call_once(flag, [&connection, this]() {
        // add our D-Bus interface and connect to D-Bus
        if (!connection->registerService(kDaemonName)) {
            fmCritical() << QString("Cannot register the \"%1\" service!!!\n").arg(kDaemonName);
            ::exit(EXIT_FAILURE);
        }

        fmInfo() << "Init DBus OperationsStackManager start";
        initOperationsDBus(connection);
        fmInfo() << "Init DBus OperationsStackManager end";

        fmInfo() << "Init DBus DeviceManager start";
        initDeviceDBus(connection);
        fmInfo() << "Init DBus DeviceManager end";
    });
}

void Core::initDeviceDBus(QDBusConnection *connection)
{
    // register object
    deviceManager.reset(new DeviceManagerDBus);
    Q_UNUSED(new DeviceManagerAdaptor(deviceManager.data()));
    if (!connection->registerObject(kDeviceManagerObjPath,
                                    deviceManager.data())) {
        fmWarning() << QString("Cannot register the \"%1\" object.\n").arg(kDeviceManagerObjPath);
        deviceManager.reset(nullptr);
    }
}

void Core::initOperationsDBus(QDBusConnection *connection)
{
    // register object
    operationsStackManager.reset(new OperationsStackManagerDbus);
    Q_UNUSED(new OperationsStackManagerAdaptor(operationsStackManager.data()));
    if (!connection->registerObject(kOperationsStackManagerObjPath,
                                    operationsStackManager.data())) {
        fmWarning() << QString("Cannot register the \"%1\" object.\n").arg(kOperationsStackManagerObjPath);
        operationsStackManager.reset(nullptr);
    }
}

void Core::exitOnShutdown(bool shutdown)
{
    if (shutdown) {
        fmInfo() << "PrepareForShutdown is emitted, exit...";
        qApp->exit(0);
    }
}

DAEMONPCORE_END_NAMESPACE
