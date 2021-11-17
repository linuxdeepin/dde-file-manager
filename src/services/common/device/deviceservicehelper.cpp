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
#include <QStandardPaths>
#include <QProcess>
#include <QMutexLocker>
#include <DDesktopServices>

Q_GLOBAL_STATIC_WITH_ARGS(dfmbase::Settings, gsGlobal, ("deepin/dde-file-manager", dfmbase::Settings::GenericConfig))

DWIDGET_USE_NAMESPACE
DSC_BEGIN_NAMESPACE

dfmbase::Settings *DeviceServiceHelper::getGsGlobal()
{
    return gsGlobal;
}

void DeviceServiceHelper::openFileManagerToDevice(const DeviceServiceHelper::BlockDevPtr &blkDev)
{
    if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
        QString root { dfmbase::UrlRoute::rootPath(dfmbase::SchemeTypes::ROOT) };
        QString mountUrlStr { root + QFileInfo(blkDev->device()).fileName() + "." + dfmbase::SuffixInfo::BLOCK };
        QProcess::startDetached(QStringLiteral("dde-file-manager"), { mountUrlStr });
        qInfo() << "open by dde-file-manager: " << mountUrlStr;
        return;
    }
    QString &&mp = blkDev->mountPoint();
    qInfo() << "a new device mount to: " << mp;
    DDesktopServices::showFolder(QUrl::fromLocalFile(mp));
}

std::once_flag &DeviceServiceHelper::autoMountOnceFlag()
{
    static std::once_flag flag;
    return flag;
}

QList<QUrl> DeviceServiceHelper::makeMountpointsForDrive(const QString &driveName)
{
    QList<QUrl> urls;

    auto blkDevcies = DeviceServiceHelper::createAllBlockDevices();

    for (const auto &device : blkDevcies) {
        if (device->drive() == driveName) {
            QUrl &&url = makeMountpointForBlock(device);
            if (url.isValid())
                urls << url;
        }
    }

    return urls;
}

QList<QUrl> DeviceServiceHelper::makeMountpointsForAllDrive()
{
    QList<QUrl> urls;
    auto blkDevcies = DeviceServiceHelper::createAllBlockDevices();

    for (const auto &device : blkDevcies) {
        QUrl &&url = makeMountpointForBlock(device);
        urls << url;
    }

    return urls;
}

QUrl DeviceServiceHelper::makeMountpointForBlock(const BlockDevPtr &blkDev)
{
    if (!blkDev)
        return QUrl();
    if (blkDev->mountPoint().isEmpty())
        return QUrl();

    return QUrl::fromLocalFile(blkDev->mountPoint());
}

QStringList DeviceServiceHelper::makeAllDevicesIdForDrive(const QString &driveName)
{
    QStringList idList;

    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    auto monitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice).staticCast<DFMMOUNT::DFMBlockMonitor>();
    idList = monitor->resolveDeviceFromDrive(driveName);

    return idList;
}

bool DeviceServiceHelper::isUnmountableBlockDevice(const DeviceServiceHelper::BlockDevPtr &blkDev)
{
    if (!blkDev || blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr";
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
    if (!blkDev || blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr";
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
    if (!blkDev || blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr";
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

    if (data.cryptoBackingDevice.length() > 1) {   // bug: 77010
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

    qInfo() << "can eject? " << data.common.id << "(mountpoints removable optical ejectable): "
            << data.mountpoints << removable << optical << ejectable;

    if (removable)
        return true;

    if (optical && ejectable)
        return true;

    return false;
}

bool DeviceServiceHelper::isCanPoweroffBlockDevice(const DeviceServiceHelper::BlockDevPtr &blkDev)
{
    if (!blkDev || blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr";
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
    bool canPowerOff = data.canPowerOff;

    qInfo() << "can poweroff? " << mpts << canPowerOff;

    if (!canPowerOff)
        return false;

    return true;
}

bool DeviceServiceHelper::isProtectedBlocDevice(const BlockDeviceData &data)
{
    QGSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");

    if (gsettings.get("protect-non-media-mounts").toBool()) {
        for (auto &mountPoint : data.mountpoints) {
            if (!mountPoint.isEmpty() && !mountPoint.startsWith("/media/"))
                return true;
        }
    }

    // Warning: monitor only work in main thread
    if (dfmbase::UniversalUtils::inMainThread() && gsettings.get("protect-root-device-mounts").toBool()) {
        QStorageInfo qsi("/");
        auto manager = DFMMOUNT::DFMDeviceManager::instance();
        auto monitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice).staticCast<DFMMOUNT::DFMBlockMonitor>();
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
    auto &&id = data.common.id;

    if (data.hasPartitionTable) {
        qDebug() << "Block device is ignored by parent node:" << id;
        return true;
    }

    if (Q_UNLIKELY(data.hintIgnore)) {
        qWarning() << "Block device is ignored by hintIgnore:" << id;
        return true;
    }

    if (Q_UNLIKELY(data.cryptoBackingDevice.length() > 1)) {
        qWarning() << "Block device is ignored by crypted back device:" << id;
        return true;
    }

    if (Q_UNLIKELY(data.isLoopDevice)) {
        qDebug() << "Block device is ignored by loop device:" << id;
        return true;
    }

    if (Q_UNLIKELY(!data.hasFileSystem && !data.isEncrypted
                   && !data.removable && !data.mediaCompatibility.join(" ").contains("optical"))) {
        qWarning() << "Block device is ignored by wrong removeable set for system disk:" << id;
        return true;
    }

    // TODO(zhangs): refrence dfmrootcontroller -> ignoreBlkDevice
    // -> get device partion type ...

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
    auto &&devList = DeviceServiceHelper::createAllDevices(DFMMOUNT::DeviceType::BlockDevice);
    for (auto dev : devList) {
        auto blkDev = dev.staticCast<DFMMOUNT::DFMBlockDevice>();
        if (blkDev)
            list.append(blkDev);
    }
    return list;
}

DeviceServiceHelper::ProtocolDevPtrList DeviceServiceHelper::createAllProtocolDevices()
{
    ProtocolDevPtrList list;
    auto &&devList = DeviceServiceHelper::createAllDevices(DFMMOUNT::DeviceType::ProtocolDevice);
    for (auto dev : devList) {
        auto protocolDev = dev.staticCast<DFMMOUNT::DFMProtocolDevice>();
        if (protocolDev)
            list.append(protocolDev);
    }
    return list;
}

void DeviceServiceHelper::makeBlockDeviceData(const DeviceServiceHelper::BlockDevPtr &ptr, BlockDeviceData *data)
{
    Q_ASSERT_X(data, "DeviceServiceHelper", "Data is NULL");

    data->common.id = ptr->path();
    data->common.filesystem = ptr->fileSystem();
    data->common.mountpoint = ptr->mountPoint();
    data->common.sizeTotal = ptr->sizeTotal();
    data->common.sizeFree = ptr->sizeFree();
    data->common.sizeUsed = ptr->sizeUsage();

    data->mountpoints = ptr->mountPoints();
    data->device = ptr->device();
    data->drive = ptr->drive();
    data->idLabel = ptr->idLabel();
    data->media = ptr->getProperty(DFMMOUNT::Property::DriveMedia).toString();
    data->mediaCompatibility = ptr->mediaCompatibility();
    data->removable = ptr->removable();
    data->optical = ptr->optical();
    data->opticalBlank = ptr->opticalBlank();
    data->mediaAvailable = ptr->getProperty(DFMMOUNT::Property::DriveMediaAvailable).toBool();
    data->canPowerOff = ptr->canPowerOff();
    data->ejectable = ptr->ejectable();
    data->isEncrypted = ptr->isEncrypted();
    data->isLoopDevice = ptr->isLoopDevice();
    data->hasFileSystem = ptr->hasFileSystem();
    data->hasPartitionTable = ptr->hasPartitionTable();
    data->hintSystem = ptr->hintSystem();
    data->hintIgnore = ptr->hintIgnore();
    data->cryptoBackingDevice = ptr->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString();
}

void DeviceServiceHelper::makeBlockDeviceMap(const BlockDeviceData &data, QVariantMap *map, bool detail)
{
    Q_ASSERT_X(map, "DeviceServiceHelper", "Map is NULL");

    map->insert("id", data.common.id);
    map->insert("filesystem", data.common.filesystem);
    map->insert("mountpoint", data.common.mountpoint);
    map->insert("size_total", data.common.sizeTotal);
    map->insert("size_free", data.common.sizeFree);
    map->insert("size_usage", data.common.sizeUsed);

    map->insert("device", data.device);
    map->insert("id_label", data.idLabel);
    map->insert("media", data.media);
    map->insert("removable", data.removable);
    map->insert("optical", data.optical);
    map->insert("optical_blank", data.opticalBlank);
    map->insert("media_available", data.mediaAvailable);
    map->insert("can_power_off", data.canPowerOff);
    map->insert("ejectable", data.ejectable);
    map->insert("is_encrypted", data.isEncrypted);
    map->insert("is_loop_device", data.isLoopDevice);
    map->insert("has_filesystem", data.hasFileSystem);
    map->insert("has_partition_table", data.hasPartitionTable);
    map->insert("hint_system", data.hintSystem);
    map->insert("hint_ignore", data.hintIgnore);
    map->insert("crypto_backingDevice", data.cryptoBackingDevice);

    // Too much information can slow down the performance of D-Bus interface calls,
    // so only return all information when using `detail`.
    if (detail) {
        map->insert("mountpoints", data.mountpoints);
        map->insert("drive", data.drive);
        map->insert("media_compatibility", data.mediaCompatibility);
    }
}

void DeviceServiceHelper::updateBlockDeviceSizeUsed(BlockDeviceData *data, qint64 total, qint64 free)
{
    if (data) {
        static QMutex mutex;
        QMutexLocker guard(&mutex);
        data->common.sizeTotal = total;
        data->common.sizeFree = free;
        data->common.sizeUsed = total - free;
    }
}

DSC_END_NAMESPACE
