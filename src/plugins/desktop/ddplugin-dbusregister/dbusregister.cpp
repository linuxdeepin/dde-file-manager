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

#include "services/common/device/deviceservice.h"
#include "devicemanagerdbus.h"
#include "dbus_adaptor/devicemanagerdbus_adaptor.h"

#include "dfm-base/base/urlroute.h"

#include <dfm-framework/framework.h>
#include <QDBusConnection>

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void DBusRegister::initialize()
{
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(DeviceService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
    UrlRoute::regScheme(SchemeTypes::kEntry, "/", QIcon(), true);
}

bool DBusRegister::start()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }

    initServiceDBusInterfaces(connection);
    return true;
}

dpf::Plugin::ShutdownFlag DBusRegister::stop()
{
    return kSync;
}

std::once_flag &DBusRegister::onceFlag()
{
    static std::once_flag flag;
    return flag;
}

void DBusRegister::initServiceDBusInterfaces(QDBusConnection &connection)
{
    std::call_once(DBusRegister::onceFlag(), [&connection, this]() {
        // add our D-Bus interface and connect to D-Bus
        if (!connection.registerService("com.deepin.filemanager.service")) {
            qWarning("Cannot register the \"com.deepin.filemanager.service\" service.\n");
            return;
        }

        // register object
        deviceManager.reset(new DeviceManagerDBus);
        Q_UNUSED(new DeviceManagerAdaptor(deviceManager.data()));
        if (!connection.registerObject("/com/deepin/filemanager/service/DeviceManager",
                                       deviceManager.data())) {
            qWarning("Cannot register the \"/com/deepin/filemanager/service/DeviceManager\" object.\n");
            deviceManager->deleteLater();
            return;
        }
    });
}
