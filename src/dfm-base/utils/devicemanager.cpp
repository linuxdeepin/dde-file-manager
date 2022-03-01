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

#include "dfm-base/base/device/devicecontroller.h"

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

    qInfo() << "Finish initilize dbus: `DeviceManagerInterface`" << isServiceDBusRunning();
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

    // both status
    if (isServiceDBusRunning())
        initDeviceServiceDBusConn();
    else
        initDeviceServiceConn();
    connect(this, &DeviceManager::serviceRegistered, this, [this]() {
        initDeviceServiceDBusConn();
        disconnDeviceService();
    });
    connect(this, &DeviceManager::serviceUnregistered, this, [this]() {
        initDeviceServiceConn();
        disconnDeviceServiceDBus();
    });
}

bool DeviceManager::isServiceDBusRunning()
{
    return deviceInterface && deviceInterface->isValid();
}

QString DeviceManager::blockDeviceId(const QString &device)
{
    static constexpr char kBlockDeviceIdPrefix[] { "/org/freedesktop/UDisks2/block_devices/" };

    QString dev { device };
    if (dev.startsWith("/dev/"))
        dev = dev.remove("/dev/");

    return QString(kBlockDeviceIdPrefix) + dev;
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

bool DeviceManager::invokeIsMonitorWorking()
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

    if (Q_LIKELY(isServiceDBusRunning())) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->GetBlockDevicesIdList(opt);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    } else {
        ret = DeviceController::instance()->blockDevicesIdList(opt);
    }

    return ret;
}

QStringList DeviceManager::invokeProtolcolDevicesIdList(const QVariantMap &opt)
{
    Q_UNUSED(opt)

    QStringList ret;

    if (Q_LIKELY(isServiceDBusRunning())) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->GetProtocolDevicesIdList();
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    } else {
        ret = DeviceController::instance()->protocolDevicesIdList();
    }

    return ret;
}

QVariantMap DeviceManager::invokeQueryBlockDeviceInfo(const QString &id, bool detail)
{
    QVariantMap ret;
    if (Q_LIKELY(isServiceDBusRunning())) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id << detail;
        auto &&reply = deviceInterface->QueryBlockDeviceInfo(id, detail);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    } else {
        ret = DeviceController::instance()->blockDeviceInfo(id, detail);
    }
    return ret;
}

QVariantMap DeviceManager::invokeQueryProtocolDeviceInfo(const QString &id, bool detail)
{
    QVariantMap ret;
    if (Q_LIKELY(isServiceDBusRunning())) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id << detail;
        auto &&reply = deviceInterface->QueryProtocolDeviceInfo(id, detail);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    } else {
        ret = DeviceController::instance()->protocolDeviceInfo(id, detail);
    }
    return ret;
}

bool DeviceManager::invokeDetachBlockDevice(const QString &id)
{
    if (Q_LIKELY(isServiceDBusRunning())) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->DetachBlockDevice(id);
        if (!reply.isValid()) {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return false;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return reply.value();
    } else {
        return DeviceController::instance()->detachBlockDevice(id);
    }
}

bool DeviceManager::invokeDetachBlockDeviceForced(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->DetachBlockDeviceForced(id);
        if (!reply.isValid()) {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return false;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return reply.value();
    }
    return false;
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

void DeviceManager::invokeGhostBlockDevMounted(const QString &id, const QString &mountPoint)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->GhostBlockDevMounted(id, mountPoint);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid " << reply.error();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

bool DeviceManager::invokeDetachProtocolDevice(const QString &id)
{
    if (Q_LIKELY(isServiceDBusRunning())) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__ << id;
        auto &&reply = deviceInterface->DetachProtocolDevice(id);
        if (!reply.isValid()) {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return false;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return reply.value();
    } else {
        return DeviceController::instance()->detachProtocolDevice(id);
    }
}

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::initDeviceServiceDBusConn()
{
    qInfo() << "connecting DBus signals...";

    connect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceAdded, this, &DeviceManager::blockDevAdded);
    connect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceRemoved, this, &DeviceManager::blockDevRemoved);
    connect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceMounted, this, &DeviceManager::blockDevMounted);
    connect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceUnmounted, this, &DeviceManager::blockDevUnmounted);
    connect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceLocked, this, &DeviceManager::blockDevLocked);
    connect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceUnlocked, this, &DeviceManager::blockDevUnlocked);
    connect(getDeviceInterface(), &DeviceManagerInterface::BlockDevicePropertyChanged, this, [this](const QString &id, const QString &property, const QDBusVariant &value) {
        emit blockDevicePropertyChanged(id, property, value.variant());
    });
    connect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceMounted, this, &DeviceManager::protocolDevMounted);
    connect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceUnmounted, this, &DeviceManager::protocolDevUnmounted);
    connect(getDeviceInterface(), &DeviceManagerInterface::SizeUsedChanged, this, &DeviceManager::deviceSizeUsedChanged);
    connect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceAdded, this, &DeviceManager::protocolDevAdded);
    connect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceRemoved, this, &DeviceManager::protocolDevRemoved);
}

void DeviceManager::disconnDeviceServiceDBus()
{
    qInfo() << "disconnecting DBus signals...";

    disconnect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceAdded, this, &DeviceManager::blockDevAdded);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceRemoved, this, &DeviceManager::blockDevRemoved);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceMounted, this, &DeviceManager::blockDevMounted);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceUnmounted, this, &DeviceManager::blockDevUnmounted);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceLocked, this, &DeviceManager::blockDevLocked);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::BlockDeviceUnlocked, this, &DeviceManager::blockDevUnlocked);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceMounted, this, &DeviceManager::protocolDevMounted);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceUnmounted, this, &DeviceManager::protocolDevUnmounted);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::SizeUsedChanged, this, &DeviceManager::deviceSizeUsedChanged);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceAdded, this, &DeviceManager::protocolDevAdded);
    disconnect(getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceRemoved, this, &DeviceManager::protocolDevRemoved);
}

void DeviceManager::initDeviceServiceConn()
{
    qInfo() << "connecting controller signals...";

    auto controller = DeviceController::instance();
    controller->startMonitor();

    connect(controller, &DeviceController::blockDevAdded, this, &DeviceManager::blockDevAdded);
    connect(controller, &DeviceController::blockDevRemoved, this, &DeviceManager::blockDevRemoved);
    connect(controller, &DeviceController::blockDevMounted, this, &DeviceManager::blockDevMounted);
    connect(controller, &DeviceController::blockDevUnmounted, this, &DeviceManager::blockDevUnmounted);
    connect(controller, &DeviceController::blockDevLocked, this, &DeviceManager::blockDevLocked);
    connect(controller, &DeviceController::blockDevUnlocked, this, &DeviceManager::blockDevUnlocked);
    connect(controller, &DeviceController::blockDevicePropertyChanged, this, &DeviceManager::blockDevicePropertyChanged);
    connect(controller, &DeviceController::protocolDevMounted, this, &DeviceManager::protocolDevMounted);
    connect(controller, &DeviceController::protocolDevUnmounted, this, &DeviceManager::protocolDevUnmounted);
    connect(controller, &DeviceController::deviceSizeUsedChanged, this, &DeviceManager::deviceSizeUsedChanged);
    connect(controller, &DeviceController::protocolDevAdded, this, &DeviceManager::protocolDevAdded);
    connect(controller, &DeviceController::protocolDevRemoved, this, &DeviceManager::protocolDevRemoved);
}

void DeviceManager::disconnDeviceService()
{
    qInfo() << "disconnecting controller signals...";

    auto controller = DeviceController::instance();
    controller->stopMonitor();

    disconnect(controller, &DeviceController::blockDevAdded, this, &DeviceManager::blockDevAdded);
    disconnect(controller, &DeviceController::blockDevRemoved, this, &DeviceManager::blockDevRemoved);
    disconnect(controller, &DeviceController::blockDevMounted, this, &DeviceManager::blockDevMounted);
    disconnect(controller, &DeviceController::blockDevUnmounted, this, &DeviceManager::blockDevUnmounted);
    disconnect(controller, &DeviceController::blockDevLocked, this, &DeviceManager::blockDevLocked);
    disconnect(controller, &DeviceController::blockDevUnlocked, this, &DeviceManager::blockDevUnlocked);
    disconnect(controller, &DeviceController::blockDevicePropertyChanged, this, &DeviceManager::blockDevicePropertyChanged);
    disconnect(controller, &DeviceController::protocolDevMounted, this, &DeviceManager::protocolDevMounted);
    disconnect(controller, &DeviceController::protocolDevUnmounted, this, &DeviceManager::protocolDevUnmounted);
    disconnect(controller, &DeviceController::deviceSizeUsedChanged, this, &DeviceManager::deviceSizeUsedChanged);
    disconnect(controller, &DeviceController::protocolDevAdded, this, &DeviceManager::protocolDevAdded);
    disconnect(controller, &DeviceController::protocolDevRemoved, this, &DeviceManager::protocolDevRemoved);
}
