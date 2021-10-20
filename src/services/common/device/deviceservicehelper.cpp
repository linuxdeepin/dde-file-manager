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
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    // TODO(zhangs): wait dfm-mount change monitor
    QList<DFMMOUNT::DFMDevice *> blkDevcies = manager->devices(DFMMOUNT::DeviceType::BlockDevice);
    for (auto *dev : blkDevcies) {
        auto blkDev = qobject_cast<DFMMOUNT::DFMBlockDevice*>(dev);
        if (!mountBlockDevice(blkDev, {{"auth.no_user_interaction", true}})) {
            qWarning() << "Mount device failed: " << blkDev->path() << static_cast<int>(blkDev->getLastError());
            continue;
        }
    }
}

bool DeviceServiceHelper::mountBlockDevice(dfmmount::DFMBlockDevice *blkDev, const QVariantMap &opts)
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

        if (hasFS && mounted && !hintIgnore && !hintSystem) {
            // umount
            blk->unmount();
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

QUrl DeviceServiceHelper::getMountPathForBlock(const dfmmount::DFMBlockDevice *blkDev)
{
    if (!blkDev)
        return QUrl();
    if (blkDev->mountPoint().isEmpty())
        return QUrl();

    return blkDev->mountPoint();
}

bool DeviceServiceHelper::isMountableBlockDevice(const dfmmount::DFMBlockDevice *blkDev)
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

bool DeviceServiceHelper::isProtectedBlocDevice(const dfmmount::DFMBlockDevice *blkDev)
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

void DeviceServiceHelper::showUnmountFailedNotification(DFMMOUNT::MountError err)
{
    qWarning() << "error happened :" <<  static_cast<int>(err);
    dfmbase::UniversalUtils::notifyMessage(QObject::tr("The device was not safely removed"),
                                           QObject::tr("Click \"Safely Remove\" and then disconnect it next time"));
}

bool DeviceServiceHelper::powerOffBlockblockDeivce(dfmmount::DFMBlockDevice *blkDev)
{
    bool removable = blkDev->getProperty(DFMMOUNT::Property::DriveRemovable).toBool();
    bool optical = blkDev->getProperty(DFMMOUNT::Property::DriveOptical).toBool();
    bool ejectable = blkDev->getProperty(DFMMOUNT::Property::DriveEjectable).toBool();
    bool canPowerOff = blkDev->getProperty(DFMMOUNT::Property::DriveCanPowerOff).toBool();
    qInfo() << "unmount(removable optical ejectable canPowerOff): "
            << blkDev->mountPoint() << removable << optical << ejectable << canPowerOff;

    if (removable) {
        blkDev->eject();
        if (blkDev->getLastError() != DFMMOUNT::MountError::NoError) {
            showUnmountFailedNotification(blkDev->getLastError());
            return false;
        }
    }

    if (optical && ejectable) {
        blkDev->eject();
        if (blkDev->getLastError() != DFMMOUNT::MountError::NoError)
            showUnmountFailedNotification(blkDev->getLastError());
        return false;
    }

    if (canPowerOff)
        blkDev->powerOff();

    return true;
}

DSC_END_NAMESPACE
