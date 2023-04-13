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

SERVERPCORE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

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
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }

    initServiceDBusInterfaces(&connection);

    if (!DevProxyMng->initService()) {
        qCritical() << "device manager cannot connect to server!";
        DevMngIns->startMonitor();
    }

    return true;
}

void Core::initServiceDBusInterfaces(QDBusConnection *connection)
{
    // TODO(zhangs): move to thread!
    static std::once_flag flag;
    std::call_once(flag, [&connection, this]() {
        // add our D-Bus interface and connect to D-Bus
        if (!connection->registerService("org.deepin.filemanager.server")) {
            qCritical("Cannot register the \"org.deepin.filemanager.server\" service!!!\n");
            ::exit(EXIT_FAILURE);
        }

        qInfo() << "Init DBus OperationsStackManager start";
        initOperationsDBus(connection);
        qInfo() << "Init DBus OperationsStackManager end";

        qInfo() << "Init DBus DeviceManager start";
        initDeviceDBus(connection);
        qInfo() << "Init DBus DeviceManager end";
    });
}

void Core::initDeviceDBus(QDBusConnection *connection)
{
    // register object
    deviceManager.reset(new DeviceManagerDBus);
    Q_UNUSED(new DeviceManagerAdaptor(deviceManager.data()));
    if (!connection->registerObject("/org/deepin/filemanager/server/DeviceManager",
                                    deviceManager.data())) {
        qWarning("Cannot register the \"/org/deepin/filemanager/server/DeviceManager\" object.\n");
        deviceManager.reset(nullptr);
    }
}

void Core::initOperationsDBus(QDBusConnection *connection)
{
    // register object
    operationsStackManager.reset(new OperationsStackManagerDbus);
    Q_UNUSED(new OperationsStackManagerAdaptor(operationsStackManager.data()));
    if (!connection->registerObject("/org/deepin/filemanager/server/OperationsStackManager",
                                    operationsStackManager.data())) {
        qWarning("Cannot register the \"/org/deepin/filemanager/server/OperationsStackManager\" object.\n");
        operationsStackManager.reset(nullptr);
    }
}

SERVERPCORE_END_NAMESPACE
