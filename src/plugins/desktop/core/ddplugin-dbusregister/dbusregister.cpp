// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusregister.h"

#include "devicemanagerdbus.h"
#include "dbus_adaptor/devicemanagerdbus_adaptor.h"
#include "dbus_adaptor/operationsstackmanagerdbus_adaptor.h"
#include "dbus_adaptor/vaultmanagerdbus_adaptor.h"
#include "dbus_adaptor/filemanager1dbus_adaptor.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/dfm_global_defines.h"

#include <dfm-framework/dpf.h>

#include <QDBusConnection>
#include <QtConcurrent>

#include <dfm-io/dfmio_utils.h>

DFMBASE_USE_NAMESPACE

void DBusRegister::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true);
    dpfSignalDispatcher->subscribe("ddplugin_core", "signal_DesktopFrame_WindowShowed", this, &DBusRegister::onWindowShowed);
    // NOTE(xust): this is used to launch GVolumeMonitor in main thread, this function obtained the GVolumeMonitor instance indirectly,
    // a GVolumeMonitor instance must run in main thread to make sure the messages about device change can be send correctly,
    // the GVolumeMonitor instance is expected to be initialized in DDeviceManager but the DDeviceManager is delay intialized when
    // main window of desktop showed. So invoke the method here to make sure the instance is initialized in main thread.
    // NOTE(xust): this may take 10ms when app launch, but no better way to solve the level-2 issue for now.
    DFMIO::DFMUtils::fileIsRemovable(QUrl::fromLocalFile("/"));
}

bool DBusRegister::start()
{
    return true;
}

void DBusRegister::registerDBus()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return;
    }

    qInfo() << "Start register DBus interfaces";
    initFreedesktopDBusInterfaces(&connection);

    QTimer::singleShot(1000, [this]() {
        // mount business
        QDBusConnection connection = QDBusConnection::sessionBus();
        initServiceDBusInterfaces(&connection);

        if (!DevProxyMng->connectToService()) {
            qCritical() << "device manager cannot connect to server!";
            DevMngIns->startMonitor();
        }
    });
}

void DBusRegister::onWindowShowed()
{
    dpfSignalDispatcher->unsubscribe("ddplugin_core", "signal_DesktopFrame_WindowShowed", this, &DBusRegister::onWindowShowed);
    registerDBus();
}

std::once_flag &DBusRegister::onceFlag()
{
    static std::once_flag flag;
    return flag;
}

void DBusRegister::initServiceDBusInterfaces(QDBusConnection *connection)
{
    static std::once_flag flag;
    std::call_once(flag, [&connection, this]() {
        // add our D-Bus interface and connect to D-Bus
        if (!connection->registerService("org.deepin.filemanager.service")) {
            qWarning("Cannot register the \"org.deepin.filemanager.service\" service.\n");
            return;
        }
        qInfo() << "Init DBus OperationsStackManager start";
        initOperationsDBus(connection);
        qInfo() << "Init DBus OperationsStackManager end";

        qInfo() << "Init DBus VaultManager start";
        initVaultDBus(connection);
        qInfo() << "Init DBus VaultManager end";

        qInfo() << "Init DBus DeviceManager start";
        initDeviceDBus(connection);
        qInfo() << "Init DBus DeviceManager end";
    });
}

void DBusRegister::initFreedesktopDBusInterfaces(QDBusConnection *connection)
{
    static std::once_flag flag;
    std::call_once(flag, [&connection, this]() {
        // add Freedesktop D-Bus interface and connect to D-Bus
        if (!connection->registerService("org.freedesktop.FileManager1")) {
            qWarning("Cannot register the \"org.freedesktop.FileManager1\" service.\n");
            return;
        }
        qInfo() << "Init DBus FileManager1 start";
        initFileManager1DBus(connection);
        qInfo() << "Init DBus FileManager1 end";
    });
}

void DBusRegister::initDeviceDBus(QDBusConnection *connection)
{
    // register object
    deviceManager.reset(new DeviceManagerDBus);
    Q_UNUSED(new DeviceManagerAdaptor(deviceManager.data()));
    if (!connection->registerObject("/org/deepin/filemanager/service/DeviceManager",
                                    deviceManager.data())) {
        qWarning("Cannot register the \"/org/deepin/filemanager/service/DeviceManager\" object.\n");
        deviceManager.reset(nullptr);
    }
}

void DBusRegister::initOperationsDBus(QDBusConnection *connection)
{
    // register object
    operationsStackManager.reset(new OperationsStackManagerDbus);
    Q_UNUSED(new OperationsStackManagerAdaptor(operationsStackManager.data()));
    if (!connection->registerObject("/org/deepin/filemanager/service/OperationsStackManager",
                                    operationsStackManager.data())) {
        qWarning("Cannot register the \"/org/deepin/filemanager/service/OperationsStackManager\" object.\n");
        operationsStackManager.reset(nullptr);
    }
}

void DBusRegister::initVaultDBus(QDBusConnection *connection)
{
    // register object
    vaultManager.reset(new VaultManagerDBus);
    Q_UNUSED(new VaultManagerAdaptor(vaultManager.data()));
    if (!connection->registerObject("/org/deepin/filemanager/service/VaultManager",
                                    vaultManager.data())) {
        qWarning("Cannot register the \"/org/deepin/filemanager/service/VaultManager\" object.\n");
        vaultManager.reset(nullptr);
    }
}

void DBusRegister::initFileManager1DBus(QDBusConnection *connection)
{
    // register object
    filemanager1.reset(new FileManager1DBus);
    Q_UNUSED(new FileManager1Adaptor(filemanager1.data()));
    if (!connection->registerObject("/org/freedesktop/FileManager1",
                                    filemanager1.data())) {
        qWarning("Cannot register the \"/org/freedesktop/FileManager1\" object.\n");
        filemanager1.reset(nullptr);
        return;
    }
}
