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

QList<QUrl> DeviceServiceHelper::getMountPathForDrive(const QString &driveName)
{
    QList<QUrl> urls;

    auto blkDevcies = DeviceServiceHelper::createAllBlockDevices();

    for (const auto &device : blkDevcies) {
        if (device->drive() == driveName) {
            QUrl &&url = getMountPathForBlock(device);
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
        QUrl &&url = getMountPathForBlock(device);
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

bool DeviceServiceHelper::isUnmountableBlockDevice(const DeviceServiceHelper::BlockDevPtr &blkDev)
{
    if (blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr by " << blkDev->path();
        return false;
    }

    BlockDeviceData data;
    DeviceServiceHelper::makeBlockDeviceData(blkDev, &data);

    if (DeviceServiceHelper::isUnmountableBlockDevice(data))
        return true;

    return false;
}

bool DeviceServiceHelper::isUnmountableBlockDevice(const BlockDeviceData &data)
{
    const QString &id = data.common.id;
    if (data.common.id.isEmpty()) {
        qWarning() << "Block Device is Null";
        return false;
    }

    if (isProtectedBlocDevice(data)) {
        qWarning() << "Block Device: " << id << " is protected device!";
        return false;
    }

    if (data.common.filesystem.isEmpty()) {
        qWarning() << "Block Device: " << id << " haven't a filesystem!";
        return false;
    }

    if (data.mountpoints.isEmpty()) {
        qWarning() << "Block Device: " << id << " not mounted!";
        return false;
    }

    if (data.hintIgnore) {
        qWarning() << "Block Device: " << id << " hintIgnore!";
        return false;
    }

    if (data.hintIgnore) {
        qWarning() << "Block Device: " << id << " hintSystem!";
        return false;
    }

    return true;
}

bool DeviceServiceHelper::isEjectableBlockDevice(const DeviceServiceHelper::BlockDevPtr &blkDev)
{
    if (blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr by " << blkDev->path();
        return false;
    }

    BlockDeviceData data;
    DeviceServiceHelper::makeBlockDeviceData(blkDev, &data);

    if (DeviceServiceHelper::isEjectableBlockDevice(data))
        return true;

    return false;
}

bool DeviceServiceHelper::isMountableBlockDevice(const DeviceServiceHelper::BlockDevPtr &blkDev)
{
    if (blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr by " << blkDev->path();
        return false;
    }

    BlockDeviceData data;
    DeviceServiceHelper::makeBlockDeviceData(blkDev, &data);

    if (DeviceServiceHelper::isMountableBlockDevice(data))
        return true;

    return false;
}

bool DeviceServiceHelper::isMountableBlockDevice(const BlockDeviceData &data)
{
    const QString &id = data.common.id;
    if (data.common.id.isEmpty()) {
        qWarning() << "Block Device is Null";
        return false;
    }

    if (data.isEncrypted) {
        qWarning() << "Block Device: " << id << " is encrypted device!";
        return false;
    }

    if (isProtectedBlocDevice(data)) {
        qWarning() << "Block Device: " << id << " is protected device!";
        return false;
    }

    if (data.cryptoBackingDevice.length() > 1) { // bug: 77010
        qWarning() << "Block Device: " << id << " cryptoDev length > 1";
        return false;
    }

    if (data.hintIgnore) {
        qWarning() << "Block Device: " << id << "hintIgnore";
        return false;
    }

    if (!data.mountpoints.isEmpty()) {
        qWarning() << "Block Device: " << id << " has mounted: ";
        return false;
    }

    if (data.common.filesystem.isEmpty()) {
        qWarning() << "Block Device: " << id << " haven't a filesystem";
        return false;
    }

    return true;
}

bool DeviceServiceHelper::isEjectableBlockDevice(const BlockDeviceData &data)
{
    bool removable = data.removable;
    bool optical = data.optical;
    bool ejectable = data.ejectable;

    qInfo() << "can eject? " << data.common.id << "(removable optical ejectable mountpoints): "
            << data.mountpoints << removable << optical << ejectable;

    if (removable)
        return true;

    if (optical && ejectable)
        return true;

    return false;
}

bool DeviceServiceHelper::isCanPoweroffBlockDevice(const DeviceServiceHelper::BlockDevPtr &blkDev)
{
    if (blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr by " << blkDev->path();
        return false;
    }

    BlockDeviceData data;
    DeviceServiceHelper::makeBlockDeviceData(blkDev, &data);

    if (DeviceServiceHelper::isCanPoweroffBlockDevice(data))
        return true;

    return false;
}

bool DeviceServiceHelper::isCanPoweroffBlockDevice(const BlockDeviceData &data)
{
    const QStringList &mpts = data.mountpoints;
    bool canPowerOff =  data.canPowerOff;

    qInfo()<< "can poweroff? " << mpts << canPowerOff;

   if (!canPowerOff)
       return false;

   return true;
}

bool DeviceServiceHelper::isProtectedBlocDevice(const BlockDeviceData &data)
{
    QGSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");

    if (gsettings.get("protect-non-media-mounts").toBool()) {
        for (auto &mountPoint : data.mountpoints) {
            if (!mountPoint.isEmpty() &&!mountPoint.startsWith("/media/")) {
                return true;
            }
        }
    }

    if (gsettings.get("protect-root-device-mounts").toBool()) {
        QStorageInfo qsi("/");
        auto manager = DFMMOUNT::DFMDeviceManager::instance();
        auto monitor = qobject_cast<QSharedPointer<DFMMOUNT::DFMBlockMonitor>>
                       (manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice));
        QStringList &&rootDevNodes = monitor->resolveDeviceNode(qsi.device(), {});
        if (!rootDevNodes.isEmpty()) {
            if (data.drive == createBlockDevice(rootDevNodes.first())->drive())
                return true;
        }
    }

    return false;
}

bool DeviceServiceHelper::isIgnorableBlockDevice(const BlockDeviceData &data)
{
    // TODO(zhangs): refrence to dfmrootcontroller.cpp -> ignoreBlkDevice
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
    auto &&devicesMap = manager->devices(type);
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

void DeviceServiceHelper::makeBlockDeviceData(const DeviceServiceHelper::BlockDevPtr &ptr, BlockDeviceData *data)
{
    Q_ASSERT_X(data, "DeviceServiceHelper", "Data is NULL");

    data->common.id            = ptr->path();
    data->common.filesystem    = ptr->fileSystem();
    data->common.mountpoint    = ptr->mountPoint().toLocalFile();
    data->common.sizeTotal     = ptr->sizeTotal();
    data->common.sizeFree      = ptr->sizeFree();
    data->common.sizeUsage     = ptr->sizeUsage();

    data->mountpoints          = ptr->mountPoints();
    data->device               = ptr->device();
    data->drive                = ptr->drive();
    data->idLabel              = ptr->idLabel();
    data->removable            = ptr->removable();
    data->optical              = ptr->optical();
    data->opticalBlank         = ptr->opticalBlank();
    data->mediaCompatibility   = ptr->mediaCompatibility();
    data->canPowerOff          = ptr->canPowerOff();
    data->ejectable            = ptr->ejectable();
    data->isEncrypted          = ptr->isEncrypted();
    data->hasFileSystem        = ptr->hasFileSystem();
    data->hintSystem           = ptr->getProperty(DFMMOUNT::Property::BlockHintSystem).toBool(); // TODO(zhangs): wait a interface
    data->hintIgnore           = ptr->hintIgnore();
    data->cryptoBackingDevice  = ptr->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString();
}

void DeviceServiceHelper::makeBlockDeviceMap(const BlockDeviceData &data, QVariantMap *map)
{
    Q_ASSERT_X(map, "DeviceServiceHelper", "Map is NULL");

    map->insert("id", data.common.id);
    map->insert("filesystem", data.common.filesystem);
    map->insert("mountpoint", data.common.mountpoint);
    map->insert("size_total", data.common.sizeTotal);
    map->insert("size_free", data.common.sizeFree);
    map->insert("size_usage", data.common.sizeUsage);

    map->insert("mountpoints", data.mountpoints);
    map->insert("device", data.device);
    map->insert("drive", data.drive);
    map->insert("id_label", data.idLabel);
    map->insert("removable", data.removable);
    map->insert("optical", data.optical);
    map->insert("optical_blank", data.opticalBlank);
    map->insert("media_compatibility", data.mediaCompatibility);
    map->insert("can_power_off", data.canPowerOff);
    map->insert("ejectable", data.ejectable);
    map->insert("is_encrypted", data.isEncrypted);
    map->insert("has_filesystem", data.hasFileSystem);
    map->insert("hint_system", data.hintSystem);
    map->insert("hint_ignore", data.hintIgnore);
    map->insert("crypto_backingDevice", data.cryptoBackingDevice);
}

DSC_END_NAMESPACE
