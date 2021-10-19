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

#include <QDebug>
#include <QGSettings>
#include <QStorageInfo>

Q_GLOBAL_STATIC_WITH_ARGS(dfmbase::Settings, gsGlobal, ("deepin/dde-file-manager", dfmbase::Settings::GenericConfig))

DSC_BEGIN_NAMESPACE

dfmbase::Settings *DeviceServiceHelper::getGsGlobal()
{
    return gsGlobal;
}

std::once_flag &DeviceServiceHelper::onceFlag()
{
    static std::once_flag flag;
    return flag;
}

void DeviceServiceHelper::mountAllBlockDevices()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    // TODO(zhangs): wait dfm-mount change monitor
    QList<DFMMOUNT::DFMDevice *> blkDevcies = manager->devices(DFMMOUNT::DeviceType::BlockDevice);
    for (auto *dev : blkDevcies) {
        if (!dev)
            continue;
        // TODO(zhangs): wait dfm-mount add isEncrypted
        bool hintIgnore = dev->getProperty(DFMMOUNT::Property::BlockHintIgnore).toBool();
        QString &&cryptoDev = dev->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString();

        if (cryptoDev.length() > 1)
            continue;
        if (hintIgnore)
            continue;

        // TODO(zhangs): wait dfm-mount change mountPoint interface
        QStringList &&mountPoints = dev->getProperty(DFMMOUNT::Property::FileSystemMountPoint).toStringList();
        bool hasFS = !dev->fileSystem().isEmpty();
        if (hasFS && mountPoints.isEmpty()) {
            QUrl &&mp = dev->mount({{"auth.no_user_interaction", true}});
            qInfo() << "Auto mount block device to: " << mp;
        }
    }
}

void DeviceServiceHelper::mountAllProtocolDevices()
{
    // TODO(zhangs): auto mount protocol devices (wait dfm-mount)
}

void DeviceServiceHelper::unmountAllBlockDevices()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    QList<DFMMOUNT::DFMDevice *> blkDevcies = manager->devices(DFMMOUNT::DeviceType::BlockDevice);
    for (auto *dev : blkDevcies) {
        DFMMOUNT::DFMBlockDevice *blk = qobject_cast<DFMMOUNT::DFMBlockDevice*>(dev);
        if (!blk)
            continue;
        if (isProtectedBlocDevice(blk))
            continue;

        bool hasFS = !blk->fileSystem().isEmpty();
        bool mounted = !blk->mountPoint().isEmpty();
        bool hintIgnore = blk->getProperty(DFMMOUNT::Property::BlockHintIgnore).toBool();
        bool hintSystem = blk->getProperty(DFMMOUNT::Property::BlockHintSystem).toBool();

        if (hasFS && mounted && hintIgnore && hintSystem) {
            // umount
            blk->unmountAsync();
            // there are other operations besides unmount
            if (!powerOffBlockblockDeivce(blk))
                continue;
        }
    }
}

void DeviceServiceHelper::unmountAllProtocolDevices()
{
    // TODO(zhangs): unmount protocal devices (wait dfm-mount)
}

QList<QUrl> DeviceServiceHelper::getMountPathForDrive(const QString &driveName)
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    QList<QUrl> urls;

    QList<DFMMOUNT::DFMDevice *> blockDevices = manager->devices(DFMMOUNT::DeviceType::BlockDevice);
    for (const auto *device : blockDevices) {
        if (device && device->getProperty(DFMMOUNT::Property::BlockDrive).toString() == driveName) {
            const QUrl &url = getMountPathForBlock(qobject_cast<const DFMMOUNT::DFMBlockDevice*>(device));
            if (url.isValid())
                urls << url;
        }
    }

    return urls;
}

QList<QUrl> DeviceServiceHelper::getMountPathForAllDrive()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    QList<QUrl> urls;

    QList<DFMMOUNT::DFMDevice *> blockDevices = manager->devices(DFMMOUNT::DeviceType::BlockDevice);
    for (const auto device : blockDevices) {
        if (!device)
            continue;
        const QUrl &url = getMountPathForBlock(qobject_cast<DFMMOUNT::DFMBlockDevice*>(device));
        urls << url;
    }

    return urls;
}

QUrl DeviceServiceHelper::getMountPathForBlock(const dfmmount::DFMBlockDevice *block)
{
    if (!block)
        return QUrl();
    if (block->mountPoint().isEmpty())
        return QUrl();

    return block->mountPoint();
}

bool DeviceServiceHelper::isProtectedBlocDevice(const dfmmount::DFMBlockDevice *block)
{
    QGSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");

    if (gsettings.get("protect-non-media-mounts").toBool()) {
        QStringList &&mountPoints = block->getProperty(DFMMOUNT::Property::FileSystemMountPoint).toStringList();
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

void DeviceServiceHelper::showUnmountFailedNotification(DFMMOUNT::MountError err)
{
    qWarning() << "error happened :" <<  static_cast<int>(err);
    dfmbase::UniversalUtils::notifyMessage(QObject::tr("The device was not safely removed"),
                                           QObject::tr("Click \"Safely Remove\" and then disconnect it next time"));
}

bool DeviceServiceHelper::powerOffBlockblockDeivce(dfmmount::DFMBlockDevice *block)
{
    bool removable = block->getProperty(DFMMOUNT::Property::DriveRemovable).toBool();
    bool optical = block->getProperty(DFMMOUNT::Property::DriveOptical).toBool();
    bool ejectable = block->getProperty(DFMMOUNT::Property::DriveEjectable).toBool();
    bool canPowerOff = block->getProperty(DFMMOUNT::Property::DriveCanPowerOff).toBool();
    qInfo() << "unmount: " << block->mountPoint() << removable << optical << ejectable << canPowerOff;

    if (removable) {
        block->ejectAsync();
        if (block->getLastError() != DFMMOUNT::MountError::NoError) {
            showUnmountFailedNotification(block->getLastError());
            return false;
        }
    }

    if (optical && ejectable) {
        block->ejectAsync();
        if (block->getLastError() != DFMMOUNT::MountError::NoError)
            showUnmountFailedNotification(block->getLastError());
        return false;
    }

    if (canPowerOff)
        block->powerOffAsync();

    return true;
}

DSC_END_NAMESPACE
