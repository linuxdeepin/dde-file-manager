/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

DFMBASE_USE_NAMESPACE

void DBusRegister::initialize()
{
    QString errStr;
    UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true);
    dpfSignalDispatcher->subscribe("ddplugin_core", "signal_DesktopFrame_WindowShowed", this, &DBusRegister::onWindowShowed);
}

bool DBusRegister::start()
{
    return true;
}

void DBusRegister::onWindowShowed()
{
    static QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return;
    }

    qInfo() << "Start register DBus interfaces";
    initFreedesktopDBusInterfaces(&connection);

    QTimer::singleShot(1000, [this]() {
        // mount business
        initServiceDBusInterfaces(&connection);

        if (!DevProxyMng->connectToService()) {
            qCritical() << "device manager cannot connect to server!";
            DevMngIns->startMonitor();
        }
    });
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
        if (!connection->registerService("com.deepin.filemanager.service")) {
            qWarning("Cannot register the \"com.deepin.filemanager.service\" service.\n");
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
    if (!connection->registerObject("/com/deepin/filemanager/service/DeviceManager",
                                    deviceManager.data())) {
        qWarning("Cannot register the \"/com/deepin/filemanager/service/DeviceManager\" object.\n");
        deviceManager.reset(nullptr);
    }
}

void DBusRegister::initOperationsDBus(QDBusConnection *connection)
{
    // register object
    operationsStackManager.reset(new OperationsStackManagerDbus);
    Q_UNUSED(new OperationsStackManagerAdaptor(operationsStackManager.data()));
    if (!connection->registerObject("/com/deepin/filemanager/service/OperationsStackManager",
                                    operationsStackManager.data())) {
        qWarning("Cannot register the \"/com/deepin/filemanager/service/OperationsStackManager\" object.\n");
        operationsStackManager.reset(nullptr);
    }
}

void DBusRegister::initVaultDBus(QDBusConnection *connection)
{
    // register object
    vaultManager.reset(new VaultManagerDBus);
    Q_UNUSED(new VaultManagerAdaptor(vaultManager.data()));
    if (!connection->registerObject("/com/deepin/filemanager/service/VaultManager",
                                    vaultManager.data())) {
        qWarning("Cannot register the \"/com/deepin/filemanager/service/VaultManager\" object.\n");
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
