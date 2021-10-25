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
#include "deviceservicehelper.h"

#include "dfm-base/utils/universalutils.h"

#include <dfm-mount/dfmblockmonitor.h>
#include <QDebug>
#include <QGSettings>
#include <QStorageInfo>

Q_GLOBAL_STATIC_WITH_ARGS(dfmbase::Settings, gsGlobal, ("deepin/dde-file-manager", dfmbase::Settings::GenericConfig))

DSC_BEGIN_NAMESPACE

dfmbase::Settings *DeviceServiceHelper::getGsGlobal()
{
    return gsGlobal;
}

std::once_flag &DeviceServiceHelper::autoMountOnceFlag()
{
    static std::once_flag flag;
    return flag;
}

std::once_flag &DeviceServiceHelper::connectOnceFlag()
{
    static std::once_flag flag;
    return flag;
}

void DeviceServiceHelper::mountAllBlockDevices()
{
    auto ptrList = DeviceServiceHelper::createAllBlockDevices();
    for (auto &blkDev : ptrList) {
        if (!mountBlockDevice(blkDev, {{"auth.no_user_interaction", true}})) {
            qWarning() << "Mount device failed: " << blkDev->path() << static_cast<int>(blkDev->getLastError());
            continue;
        }
    }
}

bool DeviceServiceHelper::mountBlockDevice(BlockDevPtr &blkDev, const QVariantMap &opts)
{
    if (!isMountableBlockDevice(blkDev))
        return false;

    QUrl &&mp = blkDev->mount(opts);
    if (mp.isEmpty())
        return false;

    return true;
}

void DeviceServiceHelper::mountAllProtocolDevices()
{
    // TODO(zhangs): auto mount protocol devices (wait dfm-mount)
}

void DeviceServiceHelper::ejectAllBlockDevices()
{
    auto blkDevcies = DeviceServiceHelper::createAllBlockDevices();
    for (auto &blk : blkDevcies) {
        if (!blk)
            continue;
        if (isProtectedBlocDevice(blk))
            continue;

        bool hasFS = !blk->fileSystem().isEmpty();
        bool mounted = !blk->mountPoint().isEmpty();
        bool hintIgnore = blk->getProperty(DFMMOUNT::Property::BlockHintIgnore).toBool();
        bool hintSystem = blk->getProperty(DFMMOUNT::Property::BlockHintSystem).toBool();

        if (hasFS && mounted && !hintIgnore && !hintSystem) {
            // umount
            blk->unmount();
            // there are other operations besides unmount
            if (!powerOffBlockblockDeivce(blk))
                continue;
        }
    }
}

void DeviceServiceHelper::ejectAllProtocolDevices()
{
    // TODO(zhangs): eject protocal devices (wait dfm-mount)
}

QList<QUrl> DeviceServiceHelper::getMountPathForDrive(const QString &driveName)
{
    QList<QUrl> urls;

    auto blkDevcies = DeviceServiceHelper::createAllBlockDevices();

    for (const auto &device : blkDevcies) {
        if (device && device->getProperty(DFMMOUNT::Property::BlockDrive).toString() == driveName) {
            const QUrl &url = getMountPathForBlock(device);
            if (url.isValid())
                urls << url;
        }
    }

    return urls;
}

QList<QUrl> DeviceServiceHelper::getMountPathForAllDrive()
{
    QList<QUrl> urls;
    auto blkDevcies = DeviceServiceHelper::createAllBlockDevices();

    for (const auto &device : blkDevcies) {
        if (!device)
            continue;
        const QUrl &url = getMountPathForBlock(device);
        urls << url;
    }

    return urls;
}

QUrl DeviceServiceHelper::getMountPathForBlock(const BlockDevPtr &blkDev)
{
    if (!blkDev)
        return QUrl();
    if (blkDev->mountPoint().isEmpty())
        return QUrl();

    return blkDev->mountPoint();
}

bool DeviceServiceHelper::isMountableBlockDevice(const BlockDevPtr &blkDev)
{
    if (!blkDev) {
        qWarning() << "Block Device is Null";
        return false;
    }

    // TODO(zhangs): wait dfm-mount add isEncrypted
    bool hintIgnore = blkDev->getProperty(DFMMOUNT::Property::BlockHintIgnore).toBool();
    QString &&cryptoDev = blkDev->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString();
    // TODO(zhangs): wait dfm-mount change mountPoint interface
    QStringList &&mountPoints = blkDev->getProperty(DFMMOUNT::Property::FileSystemMountPoint).toStringList();
    bool noFS = blkDev->fileSystem().isEmpty();

    if (isProtectedBlocDevice(blkDev)) {
        qWarning() << "Block Device: " << blkDev->path() << " is protected device!";
        return false;
    }

    if (cryptoDev.length() > 1) { // bug: 77010
        qWarning() << "Block Device: " << blkDev->path() << " cryptoDev length > 1";
        return false;
    }

    if (hintIgnore) {
        qWarning() << "Block Device: " << blkDev->path() << "hintIgnore";
        return false;
    }

    if (!mountPoints.isEmpty()) {
        qWarning() << "Block Device: " << blkDev->path() << " has mounted: " << mountPoints;
        return false;
    }

    if (noFS) {
        qWarning() << "Block Device: " << blkDev->path() << " haven't a filesystem";
        return false;
    }

    return true;
}

bool DeviceServiceHelper::isProtectedBlocDevice(const BlockDevPtr &blkDev)
{
    QGSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");

    if (gsettings.get("protect-non-media-mounts").toBool()) {
        QStringList &&mountPoints = blkDev->getProperty(DFMMOUNT::Property::FileSystemMountPoint).toStringList();
        for (auto &mountPoint : mountPoints) {
            if (!mountPoint.startsWith("/media/")) {
                return true;
            }
        }
    }

    if (gsettings.get("protect-root-device-mounts").toBool()) {
        QStorageInfo qsi("/");
        // TODO(zhangs): wait dfm-mount impl fllow functions:

//        QStringList rootDevNodes = DDiskManager::resolveDeviceNode(qsi.device(), {});
//        if (!rootDevNodes.isEmpty()) {
//            if (DDiskManager::createBlockDevice(rootDevNodes.first())->drive() == blk->drive()) {
//                return true;
//            }
//        }
    }

    return false;
}

DeviceServiceHelper::BlockDevPtr DeviceServiceHelper::createBlockDevice(const QString &devId)
{
     auto devPtr = createDevice(devId, DFMMOUNT::DeviceType::BlockDevice);
     return qobject_cast<BlockDevPtr>(devPtr);
}

DeviceServiceHelper::ProtocolDevPtr DeviceServiceHelper::createProtocolDevice(const QString &devId)
{
    auto devPtr = createDevice(devId, DFMMOUNT::DeviceType::ProtocolDevice);
    return qobject_cast<ProtocolDevPtr>(devPtr);
}

void DeviceServiceHelper::showEjectFailedNotification(DFMMOUNT::MountError err)
{
    qWarning() << "error happened :" <<  static_cast<int>(err);
    dfmbase::UniversalUtils::notifyMessage(QObject::tr("The device was not safely removed"),
                                           QObject::tr("Click \"Safely Remove\" and then disconnect it next time"));
}

bool DeviceServiceHelper::powerOffBlockblockDeivce(BlockDevPtr &blkDev)
{
    bool removable = blkDev->getProperty(DFMMOUNT::Property::DriveRemovable).toBool();
    bool optical = blkDev->getProperty(DFMMOUNT::Property::DriveOptical).toBool();
    bool ejectable = blkDev->getProperty(DFMMOUNT::Property::DriveEjectable).toBool();
    bool canPowerOff = blkDev->getProperty(DFMMOUNT::Property::DriveCanPowerOff).toBool();
    qInfo() << "eject(removable optical ejectable canPowerOff): "
            << blkDev->mountPoint() << removable << optical << ejectable << canPowerOff;

    if (removable) {
        blkDev->eject();
        if (blkDev->getLastError() != DFMMOUNT::MountError::NoError) {
            showEjectFailedNotification(blkDev->getLastError());
            return false;
        }
    }

    if (optical && ejectable) {
        blkDev->eject();
        if (blkDev->getLastError() != DFMMOUNT::MountError::NoError)
            showEjectFailedNotification(blkDev->getLastError());
        return false;
    }

    if (canPowerOff)
        blkDev->powerOff();

    return true;
}

DeviceServiceHelper::DevPtr DeviceServiceHelper::createDevice(const QString &devId, DFMMOUNT::DeviceType type)
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    auto monitor = manager->getRegisteredMonitor(type);
    Q_ASSERT_X(monitor, "DeviceServiceHelper", "dfm-mount return a NULL monitor!");
    return monitor->createDeviceById(devId);
}

DeviceServiceHelper::DevPtrList DeviceServiceHelper::createAllDevices(dfmmount::DeviceType type)
{
    DevPtrList list;
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    const auto &devicesMap = manager->devices(type);
    const QStringList &idList = devicesMap[type];
    for (const QString &id : idList) {
        auto dev = createDevice(id, type);
        if (dev)
            list.append(dev);
    }
    return list;
}

DeviceServiceHelper::BlockDevPtrList DeviceServiceHelper::createAllBlockDevices()
{
    BlockDevPtrList list;
    auto devList = DeviceServiceHelper::createAllDevices(DFMMOUNT::DeviceType::BlockDevice);
    for (auto dev : devList) {
        auto blkDev = qobject_cast<QSharedPointer<DFMMOUNT::DFMBlockDevice>>(dev);
        if (blkDev)
            list.append(blkDev);
    }
    return list;
}

DeviceServiceHelper::ProtocolDevPtrList DeviceServiceHelper::createAllProtocolDevices()
{
    ProtocolDevPtrList list;
    auto devList = DeviceServiceHelper::createAllDevices(DFMMOUNT::DeviceType::ProtocolDevice);
    for (auto dev : devList) {
        auto protocolDev = qobject_cast<QSharedPointer<DFMMOUNT::DFMProtocolDevice>>(dev);
        if (protocolDev)
            list.append(protocolDev);
    }
    return list;
}

DSC_END_NAMESPACE
