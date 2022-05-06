/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "deviceproxymanager.h"
#include "devicemanager.h"
#include "private/deviceproxymanager_p.h"

#include "dfm-base/dbusservice/dbus_interface/devicemanagerdbus_interface.h"

#include <QDBusServiceWatcher>

DFMBASE_USE_NAMESPACE
static constexpr char kDesktopService[] { "com.deepin.filemanager.service" };
static constexpr char kDevMngPath[] { "/com/deepin/filemanager/service/DeviceManager" };
static constexpr char kDevMngIFace[] { "com.deepin.filemanager.service.DeviceManager" };

const DeviceManagerInterface *DeviceProxyManager::getDBusIFace() const
{
    return d->devMngDBus.data();
}

QStringList DeviceProxyManager::getAllBlockIds(GlobalServerDefines::DeviceQueryOptions opts)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->devMngDBus->GetBlockDevicesIdList(opts);
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getAllBlockDevID(opts);
    }
}

QStringList DeviceProxyManager::getAllProtocolIds()
{
    if (d->isDBusRuning()) {
        auto &&reply = d->devMngDBus->GetProtocolDevicesIdList();
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getAllProtocolDevID();
    }
}

QVariantMap DeviceProxyManager::queryBlockInfo(const QString &id, bool reload)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->devMngDBus->QueryBlockDeviceInfo(id, reload);
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getBlockDevInfo(id, reload);
    }
}

QVariantMap DeviceProxyManager::queryProtocolInfo(const QString &id, bool reload)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->devMngDBus->QueryProtocolDeviceInfo(id, reload);
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getProtocolDevInfo(id, reload);
    }
}

void DeviceProxyManager::detachBlockDevice(const QString &id)
{
    if (d->isDBusRuning())
        d->devMngDBus->DetachBlockDevice(id);
    else
        DevMngIns->detachBlockDev(id);
}

void DeviceProxyManager::detachProtocolDevice(const QString &id)
{
    if (d->isDBusRuning())
        d->devMngDBus->DetachProtocolDevice(id);
    else
        DevMngIns->detachProtoDev(id);
}

void DeviceProxyManager::detachAllDevices()
{
    if (d->isDBusRuning()) {
        d->devMngDBus->DetachAllMountedDevices();
    } else {
        DevMngIns->detachAllRemovableBlockDevs();
        DevMngIns->detachAllProtoDevs();
    }
}

void DeviceProxyManager::reloadOpticalInfo(const QString &id)
{
    if (d->isDBusRuning())
        queryBlockInfo(id, true);
    else
        DevMngIns->getBlockDevInfo(id, true);
}

bool DeviceProxyManager::connectToService()
{
    qInfo() << "Start initilize dbus: `DeviceManagerInterface`";
    d->devMngDBus.reset(new DeviceManagerInterface(kDesktopService, kDevMngPath, QDBusConnection::sessionBus(), this));
    d->initConnection();
    return d->isDBusRuning();
}

bool DeviceProxyManager::isMonitorWorking()
{
    auto &&reply = d->devMngDBus->IsMonotorWorking();
    reply.waitForFinished();
    return reply.value();
}

DeviceProxyManager::DeviceProxyManager(QObject *parent)
    : QObject(parent), d(new DeviceProxyManagerPrivate(this))
{
}

DeviceProxyManager::~DeviceProxyManager()
{
}

DeviceProxyManagerPrivate::DeviceProxyManagerPrivate(DeviceProxyManager *qq)
    : q(qq)
{
}

DeviceProxyManagerPrivate::~DeviceProxyManagerPrivate()
{
}

bool DeviceProxyManagerPrivate::isDBusRuning()
{
    return devMngDBus && devMngDBus->isValid();
}

void DeviceProxyManagerPrivate::initConnection()
{
    dbusWatcher.reset(new QDBusServiceWatcher(kDesktopService, QDBusConnection::sessionBus()));
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceRegistered, q, [this] {
        connectToDBus();
        emit q->devMngDBusRegistered();
    });
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceUnregistered, q, [this] {
        connectToAPI();
        emit q->devMngDBusUnregistered();
    });

    if (isDBusRuning())
        connectToDBus();
    else
        connectToAPI();
}

void DeviceProxyManagerPrivate::connectToDBus()
{
    if (currentConnectionType == kDBusConnecting)
        return;
    disconnCurrentConnections();

    auto ptr = devMngDBus.data();
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDriveAdded, q, &DeviceProxyManager::blockDriveAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDriveRemoved, q, &DeviceProxyManager::blockDriveRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceAdded, q, &DeviceProxyManager::blockDevAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceRemoved, q, &DeviceProxyManager::blockDevRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceMounted, q, &DeviceProxyManager::blockDevMounted);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceUnmounted, q, &DeviceProxyManager::blockDevUnmounted);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceLocked, q, &DeviceProxyManager::blockDevLocked);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceUnlocked, q, &DeviceProxyManager::blockDevUnlocked);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceFilesystemAdded, q, &DeviceProxyManager::blockDevFsAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceFilesystemRemoved, q, &DeviceProxyManager::blockDevFsRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDevicePropertyChanged, q, [this](const QString &id, const QString &property, const QDBusVariant &value) {
        emit this->q->blockDevPropertyChanged(id, property, value.variant());
    });

    connections << q->connect(ptr, &DeviceManagerInterface::SizeUsedChanged, q, &DeviceProxyManager::devSizeChanged);

    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceAdded, q, &DeviceProxyManager::protocolDevAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceRemoved, q, &DeviceProxyManager::protocolDevRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceMounted, q, &DeviceProxyManager::protocolDevMounted);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceUnmounted, q, &DeviceProxyManager::protocolDevUnmounted);

    currentConnectionType = kDBusConnecting;
}

void DeviceProxyManagerPrivate::connectToAPI()
{
    if (currentConnectionType == kAPIConnecting)
        return;
    disconnCurrentConnections();

    auto ptr = DevMngIns;
    connections << q->connect(ptr, &DeviceManager::blockDriveAdded, q, &DeviceProxyManager::blockDriveAdded);
    connections << q->connect(ptr, &DeviceManager::blockDriveRemoved, q, &DeviceProxyManager::blockDriveRemoved);
    connections << q->connect(ptr, &DeviceManager::blockDevAdded, q, &DeviceProxyManager::blockDevAdded);
    connections << q->connect(ptr, &DeviceManager::blockDevRemoved, q, &DeviceProxyManager::blockDevRemoved);
    connections << q->connect(ptr, &DeviceManager::blockDevMounted, q, &DeviceProxyManager::blockDevMounted);
    connections << q->connect(ptr, &DeviceManager::blockDevUnmounted, q, &DeviceProxyManager::blockDevUnmounted);
    connections << q->connect(ptr, &DeviceManager::blockDevLocked, q, &DeviceProxyManager::blockDevLocked);
    connections << q->connect(ptr, &DeviceManager::blockDevUnlocked, q, &DeviceProxyManager::blockDevUnlocked);
    connections << q->connect(ptr, &DeviceManager::blockDevFsAdded, q, &DeviceProxyManager::blockDevFsAdded);
    connections << q->connect(ptr, &DeviceManager::blockDevFsRemoved, q, &DeviceProxyManager::blockDevFsRemoved);
    connections << q->connect(ptr, &DeviceManager::blockDevPropertyChanged, q, &DeviceProxyManager::blockDevPropertyChanged);

    connections << q->connect(ptr, &DeviceManager::devSizeChanged, q, &DeviceProxyManager::devSizeChanged);

    connections << q->connect(ptr, &DeviceManager::protocolDevAdded, q, &DeviceProxyManager::protocolDevAdded);
    connections << q->connect(ptr, &DeviceManager::protocolDevRemoved, q, &DeviceProxyManager::protocolDevRemoved);
    connections << q->connect(ptr, &DeviceManager::protocolDevMounted, q, &DeviceProxyManager::protocolDevMounted);
    connections << q->connect(ptr, &DeviceManager::protocolDevUnmounted, q, &DeviceProxyManager::protocolDevUnmounted);

    currentConnectionType = kAPIConnecting;

    DevMngIns->startMonitor();
}

void DeviceProxyManagerPrivate::disconnCurrentConnections()
{
    for (const auto &connection : connections)
        q->disconnect(connection);
    connections.clear();
    currentConnectionType = kNoneConnection;
}
