/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "devicemanager.h"
#include "global_server_defines.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

DFMBASE_USE_NAMESPACE

/*!
 * \class PluginSidecar
 * \brief PluginSidecar is wrapper for a series of D-Bus intefaces
 */

DeviceManager &DeviceManager::instance()
{
    static DeviceManager ins;
    return ins;
}

QPointer<DeviceManagerInterface> DeviceManager::getDeviceInterface()
{
    return deviceInterface.data();
}

bool DeviceManager::connectToServer()
{
    qInfo() << "Start initilize dbus: `DeviceManagerInterface`";
    // Note: the plugin depends on `dde-file-manager-server`!
    // the plugin will not work if `dde-file-manager-server` not run.
    deviceInterface.reset(new DeviceManagerInterface(
            "com.deepin.filemanager.service",
            "/com/deepin/filemanager/service/DeviceManager",
            QDBusConnection::sessionBus(),
            this));

    initConnection();

    qInfo() << "Finish initilize dbus: `DeviceManagerInterface`";
    return true;
}

void DeviceManager::initConnection()
{
    // hanlder server signals
    // method refrecent to `DiskControlWidget::onAskStopScanning`
    connect(deviceInterface.data(), &DeviceManagerInterface::AskStopScanningWhenDetach, this, [this](const QString &id) {
        emit askStopScanning("detach", id);
    });

    connect(deviceInterface.data(), &DeviceManagerInterface::AskStopScanningWhenDetachAll, this, [this]() {
        emit askStopScanning("detach_all", "");
    });

    connect(deviceInterface.data(), &DeviceManagerInterface::AskStopSacnningWhenUnmount, this, [this](const QString &id) {
        emit askStopScanning("unmount", id);
    });

    // monitor server status
    watcher.reset(new QDBusServiceWatcher("com.deepin.filemanager.service", deviceInterface->connection()));
    connect(watcher.data(), &QDBusServiceWatcher::serviceUnregistered, this, &DeviceManager::serviceUnregistered);
    connect(watcher.data(), &QDBusServiceWatcher::serviceRegistered, this, [this](const QString &service) {
        QTimer::singleShot(1000, this, [this, service] { emit this->serviceRegistered(service); });
    });
}

void DeviceManager::invokeDetachAllMountedDevices()
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->DetachAllMountedDevices();
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void DeviceManager::invokeDetachAllMountedDevicesForced()
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->DetachAllMountedDevicesForced();
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

bool DeviceManager::invokeIsMonotorWorking()
{
    bool ret = false;
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->IsMonotorWorking();
        reply.waitForFinished();
        if (reply.isValid() && reply.value())
            ret = true;
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
    return ret;
}

QStringList DeviceManager::invokeBlockDevicesIdList(const QVariantMap &opt)
{
    QStringList ret;

    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->GetBlockDevicesIdList(opt);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }

    return ret;
}

QStringList DeviceManager::invokeProtolcolDevicesIdList(const QVariantMap &opt)
{
    QStringList ret;

    // TODO(zhangs): opt
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->GetProtocolDevicesIdList();
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }

    return ret;
}

QVariantMap DeviceManager::invokeQueryBlockDeviceInfo(const QString &id, bool detail)
{
    QVariantMap ret;
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id << detail;
        auto &&reply = deviceInterface->QueryBlockDeviceInfo(id, detail);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
    return ret;
}

QVariantMap DeviceManager::invokeQueryProtocolDeviceInfo(const QString &id, bool detail)
{
    QVariantMap ret;
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id << detail;
        auto &&reply = deviceInterface->QueryProtocolDeviceInfo(id, detail);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
    return ret;
}

void DeviceManager::invokeDetachBlockDevice(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->DetachBlockDevice(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void DeviceManager::invokeDetachBlockDeviceForced(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->DetachBlockDeviceForced(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void DeviceManager::invokeUnmountBlockDeviceForced(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->UnmountBlockDeviceForced(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void DeviceManager::invokeDetachProtocolDevice(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->DetachProtocolDevice(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
}

DeviceManager::~DeviceManager()
{
}
