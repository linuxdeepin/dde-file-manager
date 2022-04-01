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
#include "devicecontrollerhelper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/utils/finallyutil.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <dfm-mount/dfmblockmonitor.h>

#include <QDebug>
#include <QGSettings/QGSettings>
#include <QStorageInfo>
#include <QStandardPaths>
#include <QProcess>
#include <QMutexLocker>
#include <DDesktopServices>

Q_GLOBAL_STATIC_WITH_ARGS(DFMBASE_NAMESPACE::Settings, gsGlobal, ("deepin/dde-file-manager", DFMBASE_NAMESPACE::Settings::GenericConfig))

static constexpr char kBurnAttribute[] { "BurnAttribute" };
static constexpr char kBurnTotalSize[] { "BurnTotalSize" };
static constexpr char kBurnUsedSize[] { "BurnUsedSize" };
static constexpr char kBurnMediaType[] { "BurnMediaType" };
static constexpr char kBurnWriteSpeed[] { "BurnWriteSpeede" };

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

using DFMBASE_NAMESPACE::FinallyUtil;
using namespace GlobalServerDefines;

DFMBASE_NAMESPACE::Settings *DeviceControllerHelper::getGsGlobal()
{
    return gsGlobal;
}

void DeviceControllerHelper::openFileManagerToDevice(const DeviceControllerHelper::BlockDevPtr &blkDev)
{
    if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
        QString root { DFMBASE_NAMESPACE::UrlRoute::rootPath(DFMBASE_NAMESPACE::Global::kEntry) };
        QString mountUrlStr { /*root + QFileInfo(blkDev->device()).fileName() + "." + DFMBASE_NAMESPACE::SuffixInfo::kBlock */ };   // TODO(xust)
        QProcess::startDetached(QStringLiteral("dde-file-manager"), { mountUrlStr });
        qInfo() << "open by dde-file-manager: " << mountUrlStr;
        return;
    }
    QString &&mp = blkDev->mountPoint();
    qInfo() << "a new device mount to: " << mp;
    // TODO(xust) seperate server with GUI
    DDesktopServices::showFolder(QUrl::fromLocalFile(mp));
}

std::once_flag &DeviceControllerHelper::autoMountOnceFlag()
{
    static std::once_flag flag;
    return flag;
}

QList<QUrl> DeviceControllerHelper::makeMountpointsForDrive(const QString &driveName)
{
    QList<QUrl> urls;

    auto blkDevcies = DeviceControllerHelper::createAllBlockDevices();

    for (const auto &device : blkDevcies) {
        if (device->drive() == driveName) {
            QUrl &&url = makeMountpointForBlock(device);
            if (url.isValid())
                urls << url;
        }
    }

    return urls;
}

QList<QUrl> DeviceControllerHelper::makeMountpointsForAllDrive()
{
    QList<QUrl> urls;
    auto blkDevcies = DeviceControllerHelper::createAllBlockDevices();

    for (const auto &device : blkDevcies) {
        QUrl &&url = makeMountpointForBlock(device);
        urls << url;
    }

    return urls;
}

QUrl DeviceControllerHelper::makeMountpointForBlock(const BlockDevPtr &blkDev)
{
    if (!blkDev)
        return QUrl();
    if (blkDev->mountPoint().isEmpty())
        return QUrl();

    return QUrl::fromLocalFile(blkDev->mountPoint());
}

QStringList DeviceControllerHelper::makeAllDevicesIdForDrive(const QString &driveName)
{
    QStringList idList;

    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    auto monitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice).staticCast<DFMMOUNT::DFMBlockMonitor>();
    idList = monitor->resolveDeviceFromDrive(driveName);

    return idList;
}

bool DeviceControllerHelper::isUnmountableBlockDevice(const DeviceControllerHelper::BlockDevPtr &blkDev, QString *errMsg)
{
    if (!blkDev || blkDev.isNull()) {
        if (errMsg)
            *errMsg = "Cannot create block device ptr";
        return false;
    }

    BlockDeviceData data;
    DeviceControllerHelper::makeBlockDeviceData(blkDev, &data);

    bool ret { false };
    QString error;
    if (DeviceControllerHelper::isUnmountableBlockDevice(data, &error))
        ret = true;

    if (errMsg)
        *errMsg = error;

    return ret;
}

bool DeviceControllerHelper::isUnmountableBlockDevice(const BlockDeviceData &data, QString *errMsg)
{
    QString error;
    FinallyUtil finally([&]() { if (errMsg) *errMsg = error; });
    const QString &id = data.common.id;

    if (data.common.id.isEmpty()) {
        error = "Block Device is Null";
        return false;
    }

    if (isProtectedBlocDevice(data)) {
        error = QString("Block Device: %1 is protected device!").arg(id);
        return false;
    }

    finally.dismiss();
    return true;
}

bool DeviceControllerHelper::isEjectableBlockDevice(const DeviceControllerHelper::BlockDevPtr &blkDev)
{
    if (!blkDev || blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr";
        return false;
    }

    BlockDeviceData data;
    DeviceControllerHelper::makeBlockDeviceData(blkDev, &data);

    if (DeviceControllerHelper::isEjectableBlockDevice(data))
        return true;

    return false;
}

bool DeviceControllerHelper::isMountableBlockDevice(const DeviceControllerHelper::BlockDevPtr &blkDev, QString *errMsg)
{
    if (!blkDev || blkDev.isNull()) {
        if (errMsg)
            *errMsg = "Cannot create block device ptr";
        return false;
    }

    BlockDeviceData data;
    DeviceControllerHelper::makeBlockDeviceData(blkDev, &data);

    bool ret { false };
    QString error;
    if (DeviceControllerHelper::isMountableBlockDevice(data, &error))
        ret = true;

    if (errMsg)
        *errMsg = error;

    return ret;
}

bool DeviceControllerHelper::isMountableBlockDevice(const BlockDeviceData &data, QString *errMsg)
{
    QString error;
    FinallyUtil finally([&]() { if (errMsg) *errMsg = error; });
    const QString &id = data.common.id;

    if (data.common.id.isEmpty()) {
        error = "Block Device is Null";
        return false;
    }

    if (data.isEncrypted) {
        error = QString("Block Device: %1 is encrypted device!").arg(id);
        return false;
    }

    if (isProtectedBlocDevice(data)) {
        error = QString("Block Device: %1 is protected device!").arg(id);
        return false;
    }

    if (data.hintIgnore) {
        error = QString("Block Device: %1 hintIgnore").arg(id);
        return false;
    }

    if (!data.mountpoints.isEmpty()) {
        error = QString("Block Device: %1 has mounted").arg(id);
        return false;
    }

    if (data.common.filesystem.isEmpty()) {
        error = QString("Block Device: %1 haven't a filesystem").arg(id);
        return false;
    }

    finally.dismiss();
    return true;
}

bool DeviceControllerHelper::isEjectableBlockDevice(const BlockDeviceData &data)
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

bool DeviceControllerHelper::isCanPoweroffBlockDevice(const DeviceControllerHelper::BlockDevPtr &blkDev)
{
    if (!blkDev || blkDev.isNull()) {
        qWarning() << "Cannot create block device ptr";
        return false;
    }

    BlockDeviceData data;
    DeviceControllerHelper::makeBlockDeviceData(blkDev, &data);

    if (DeviceControllerHelper::isCanPoweroffBlockDevice(data))
        return true;

    return false;
}

bool DeviceControllerHelper::isCanPoweroffBlockDevice(const BlockDeviceData &data)
{
    const QStringList &mpts = data.mountpoints;
    bool canPowerOff = data.canPowerOff;

    qInfo() << "can poweroff? " << mpts << canPowerOff;

    if (!canPowerOff)
        return false;

    return true;
}

bool DeviceControllerHelper::isProtectedBlocDevice(const BlockDeviceData &data)
{
    QGSettings gsettings("com.deepin.dde.dock.module.disk-mount", "/com/deepin/dde/dock/module/disk-mount/");

    if (gsettings.get("protect-non-media-mounts").toBool()) {
        for (auto mountPoint : data.mountpoints) {
            if (!mountPoint.isEmpty() && !mountPoint.endsWith("/"))
                mountPoint.append("/");

            if (!mountPoint.isEmpty() && !mountPoint.startsWith("/media/")) return true;
        }
    }

    // Warning: monitor only work in main thread
    if (DFMBASE_NAMESPACE::UniversalUtils::inMainThread() && gsettings.get("protect-root-device-mounts").toBool()) {
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

bool DeviceControllerHelper::isIgnorableBlockDevice(const BlockDeviceData &data, QString *errMsg)
{
    QString error;
    FinallyUtil finally([&]() { if (errMsg) *errMsg = error; });
    auto &&id = data.common.id;

    if (data.hasPartitionTable) {
        error = QString("Block device is ignored by parent node: %1").arg(id);
        return true;
    }

    if (Q_UNLIKELY(data.hintIgnore)) {
        error = QString("Block device is ignored by hintIgnore: %1").arg(id);
        return true;
    }

    if (Q_UNLIKELY(data.cryptoBackingDevice.length() > 1)) {
        error = QString("Block device is ignored by crypted back device: %1").arg(id);
        return true;
    }

    if (Q_UNLIKELY(data.isLoopDevice)) {
        error = QString("Block device is ignored by loop device: %1").arg(id);
        return true;
    }

    if (Q_UNLIKELY(!data.hasFileSystem && !data.isEncrypted
                   && !data.removable && !data.mediaCompatibility.join(" ").contains("optical"))) {
        error = QString("Block device is ignored by wrong removeable set for system disk: %1").arg(id);
        return true;
    }

    if (data.hasPartition && data.hasExtendedPartition) {
        error = QString("Block device is ignored by extended partion type");
        return true;
    }

    finally.dismiss();

    return false;
}

bool DeviceControllerHelper::isMountedEncryptedDevice(const BlockDeviceData &data)
{
    if (!data.isEncrypted || data.cleartextDevice.length() == 1)
        return false;

    auto clearBlkPtr = createBlockDevice(data.cleartextDevice);
    if (!clearBlkPtr) {
        qDebug() << "cannot create device pointer of unlocked device: " << data.cleartextDevice;
        return false;
    }

    BlockDeviceData clearBlkData;
    makeBlockDeviceData(clearBlkPtr, &clearBlkData);
    return !clearBlkData.mountpoints.isEmpty();
}

DeviceControllerHelper::BlockDevPtr DeviceControllerHelper::createBlockDevice(const QString &devId)
{
    auto devPtr = createDevice(devId, DFMMOUNT::DeviceType::BlockDevice);
    return qobject_cast<BlockDevPtr>(devPtr);
}

DeviceControllerHelper::ProtocolDevPtr DeviceControllerHelper::createProtocolDevice(const QString &devId)
{
    auto devPtr = createDevice(devId, DFMMOUNT::DeviceType::ProtocolDevice);
    return qobject_cast<ProtocolDevPtr>(devPtr);
}

DeviceControllerHelper::DevPtr DeviceControllerHelper::createDevice(const QString &devId, DFMMOUNT::DeviceType type)
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    auto monitor = manager->getRegisteredMonitor(type);
    Q_ASSERT_X(monitor, "DeviceServiceHelper", "dfm-mount return a NULL monitor!");
    return monitor->createDeviceById(devId);
}

DeviceControllerHelper::DevPtrList DeviceControllerHelper::createAllDevices(DFMMOUNT::DeviceType type)
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

DeviceControllerHelper::BlockDevPtrList DeviceControllerHelper::createAllBlockDevices()
{
    BlockDevPtrList list;
    auto &&devList = DeviceControllerHelper::createAllDevices(DFMMOUNT::DeviceType::BlockDevice);
    for (auto dev : devList) {
        auto blkDev = dev.staticCast<DFMMOUNT::DFMBlockDevice>();
        if (blkDev)
            list.append(blkDev);
    }
    return list;
}

DeviceControllerHelper::ProtocolDevPtrList DeviceControllerHelper::createAllProtocolDevices()
{
    ProtocolDevPtrList list;
    auto &&devList = DeviceControllerHelper::createAllDevices(DFMMOUNT::DeviceType::ProtocolDevice);
    for (auto dev : devList) {
        auto protocolDev = dev.staticCast<DFMMOUNT::DFMProtocolDevice>();
        if (protocolDev)
            list.append(protocolDev);
    }
    return list;
}

void DeviceControllerHelper::makeBlockDeviceData(const DeviceControllerHelper::BlockDevPtr &ptr, BlockDeviceData *data)
{
    Q_ASSERT_X(data, "DeviceServiceHelper", "Data is NULL");

    data->common.id = ptr->path();
    data->common.filesystem = ptr->fileSystem();
    data->common.mountpoint = ptr->mountPoint();
    data->common.sizeTotal = ptr->sizeTotal();
    data->common.sizeFree = ptr->sizeFree();
    data->common.sizeUsed = ptr->sizeUsage();

    data->uuid = ptr->getProperty(DFMMOUNT::Property::BlockIDUUID).toString();
    data->fsVersion = ptr->getProperty(DFMMOUNT::Property::BlockIDVersion).toString();
    data->mountpoints = ptr->mountPoints();
    data->device = ptr->device();
    data->drive = ptr->drive();
    data->idLabel = ptr->idLabel();
    data->media = ptr->getProperty(DFMMOUNT::Property::DriveMedia).toString();
    data->mediaCompatibility = ptr->mediaCompatibility();
    data->readOnly = ptr->getProperty(DFMMOUNT::Property::BlockReadOnly).toBool();
    data->removable = ptr->removable();
    data->mediaRemovable = ptr->getProperty(DFMMOUNT::Property::DriveMediaRemovable).toBool();
    data->optical = ptr->optical();
    data->opticalDrive = ptr->mediaCompatibility().join(" ").contains("optical");
    data->opticalBlank = ptr->opticalBlank();
    data->mediaAvailable = ptr->getProperty(DFMMOUNT::Property::DriveMediaAvailable).toBool();
    data->canPowerOff = ptr->canPowerOff();
    data->ejectable = ptr->ejectable();
    data->isEncrypted = ptr->isEncrypted();
    data->isLoopDevice = ptr->isLoopDevice();
    data->hasFileSystem = ptr->hasFileSystem();
    data->hasPartitionTable = ptr->hasPartitionTable();
    data->hasPartition = ptr->hasPartition();
    data->connectionBus = ptr->getProperty(DFMMOUNT::Property::DriveConnectionBus).toString();

    // We cannot acqurie optical capacity by udisks2
    if (data->optical && !data->mountpoints.isEmpty()) {
        BlockDeviceData buf;
        DeviceControllerHelper::readOpticalProperty(data->device, &buf);
        data->common.sizeTotal = buf.common.sizeTotal;
        data->common.sizeUsed = buf.common.sizeUsed;
        data->common.sizeFree = data->common.sizeTotal - data->common.sizeUsed;
        data->opticalMediaType = buf.opticalMediaType;
        data->opticalWriteSpeed = buf.opticalWriteSpeed;
    }

    /*!
     * brief Extended partition with CHS addressing.
     * It must reside within the first physical 8 GB of disk,
     * else use 0Fh instead (see 0Fh, 85h, C5h, D5h)
     */
    auto eType = ptr->partitionEType();
    data->hasExtendedPartition = (eType == DFMMOUNT::PartitionType::MbrWin95_Extended_LBA
                                  || eType == DFMMOUNT::PartitionType::MbrLinux_extended
                                  || eType == DFMMOUNT::PartitionType::MbrExtended
                                  || eType == DFMMOUNT::PartitionType::MbrDRDOS_sec_extend
                                  || eType == DFMMOUNT::PartitionType::MbrMultiuser_DOS_extend);

    data->hintSystem = ptr->hintSystem();
    data->hintIgnore = ptr->hintIgnore();
    data->cryptoBackingDevice = ptr->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString();
    data->cleartextDevice = ptr->getProperty(DFMMOUNT::Property::EncryptedCleartextDevice).toString();
}

void DeviceControllerHelper::makeBlockDeviceMap(const BlockDeviceData &data, QVariantMap *map, bool detail)
{
    Q_ASSERT_X(map, "DeviceServiceHelper", "Map is NULL");

    map->insert(DeviceProperty::kId, data.common.id);
    map->insert(DeviceProperty::kMountPoint, data.common.mountpoint);
    map->insert(DeviceProperty::kFileSystem, data.common.filesystem);
    map->insert(DeviceProperty::kSizeTotal, data.common.sizeTotal);
    map->insert(DeviceProperty::kSizeFree, data.common.sizeFree);
    map->insert(DeviceProperty::kSizeUsed, data.common.sizeUsed);

    map->insert(DeviceProperty::kUUID, data.uuid);
    map->insert(DeviceProperty::kFsVersion, data.fsVersion);
    map->insert(DeviceProperty::kDevice, data.device);
    map->insert(DeviceProperty::kIdLabel, data.idLabel);
    map->insert(DeviceProperty::kMedia, data.media);
    map->insert(DeviceProperty::kReadOnly, data.readOnly);
    map->insert(DeviceProperty::kRemovable, data.removable);
    map->insert(DeviceProperty::kMediaRemovable, data.mediaRemovable);
    map->insert(DeviceProperty::kOptical, data.optical);
    map->insert(DeviceProperty::kOpticalDrive, data.opticalDrive);
    map->insert(DeviceProperty::kOpticalBlank, data.opticalBlank);
    map->insert(DeviceProperty::kMediaAvailable, data.mediaAvailable);
    map->insert(DeviceProperty::kCanPowerOff, data.canPowerOff);
    map->insert(DeviceProperty::kEjectable, data.ejectable);
    map->insert(DeviceProperty::kIsEncrypted, data.isEncrypted);
    map->insert(DeviceProperty::kIsLoopDevice, data.isLoopDevice);
    map->insert(DeviceProperty::kHasFileSystem, data.hasFileSystem);
    map->insert(DeviceProperty::kHasPartitionTable, data.hasPartitionTable);
    map->insert(DeviceProperty::kHasPartition, data.hasPartition);
    map->insert(DeviceProperty::kHasExtendedPatition, data.hasExtendedPartition);
    map->insert(DeviceProperty::kHintSystem, data.hintSystem);
    map->insert(DeviceProperty::kHintIgnore, data.hintIgnore);
    map->insert(DeviceProperty::kCryptoBackingDevice, data.cryptoBackingDevice);
    map->insert(DeviceProperty::kConnectionBus, data.connectionBus);

    // Too much information can slow down the performance of D-Bus interface calls,
    // so only return all information when using `detail`.
    if (detail) {
        map->insert(DeviceProperty::kDrive, data.drive);
        map->insert(DeviceProperty::kMountPoints, data.mountpoints);
        map->insert(DeviceProperty::kMediaCompatibility, data.mediaCompatibility);
        map->insert(DeviceProperty::kCleartextDevice, data.cleartextDevice);
        if (data.optical) {
            map->insert(DeviceProperty::kOpticalMediaType, int(data.opticalMediaType));
            map->insert(DeviceProperty::kOpticalWriteSpeed, data.opticalWriteSpeed);
        }
    }
}
void DeviceControllerHelper::updateBlockDeviceSizeUsed(BlockDeviceData *data, qint64 total, qint64 free)
{
    if (data) {
        static QMutex mutex;
        QMutexLocker guard(&mutex);
        data->common.sizeTotal = total;
        data->common.sizeFree = free;
        data->common.sizeUsed = total - free;
    }
}

void DeviceControllerHelper::makeProtocolDeviceData(const DeviceControllerHelper::ProtocolDevPtr &ptr, ProtocolDeviceData *data)
{
    Q_ASSERT_X(data, "DeviceServiceHelper", "Data is NULL");
    if (!ptr)
        return;

    data->common.id = ptr->path();
    data->common.filesystem = ptr->fileSystem();
    data->common.sizeTotal = ptr->sizeTotal();
    data->common.sizeUsed = ptr->sizeUsage();
    data->common.sizeFree = data->common.sizeTotal - data->common.sizeUsed;
    data->common.mountpoint = ptr->mountPoint();
    data->displayName = ptr->displayName();
    data->deviceIcons = ptr->deviceIcons();
}

void DeviceControllerHelper::makeProtocolDeviceMap(const ProtocolDeviceData &data, QVariantMap *map, bool detail)
{
    Q_ASSERT_X(map, "DeviceServiceHelper", "Map is NULL");
    Q_UNUSED(detail);

    map->insert(DeviceProperty::kId, data.common.id);
    map->insert(DeviceProperty::kMountPoint, data.common.mountpoint);
    map->insert(DeviceProperty::kFileSystem, data.common.filesystem);
    map->insert(DeviceProperty::kSizeTotal, data.common.sizeTotal);
    map->insert(DeviceProperty::kSizeFree, data.common.sizeFree);
    map->insert(DeviceProperty::kSizeUsed, data.common.sizeUsed);

    map->insert(DeviceProperty::kDisplayName, data.displayName);
    map->insert(DeviceProperty::kDeviceIcon, data.deviceIcons);
}

void DeviceControllerHelper::updateProtocolDeviceSizeUsed(ProtocolDeviceData *data, qint64 total, qint64 free, qint64 used)
{
    if (data) {
        static QMutex mutex;
        QMutexLocker guard(&mutex);
        data->common.sizeTotal = total;
        data->common.sizeFree = free;
        data->common.sizeUsed = used;
    }
}

void DeviceControllerHelper::writeOpticalProperty(const BlockDeviceData &data)
{
    Q_ASSERT(data.device.startsWith("/dev"));

    QMap<QString, QVariant> info;
    QString tag { data.device.mid(5) };

    info[kBurnTotalSize] = data.common.sizeTotal;
    info[kBurnUsedSize] = data.common.sizeUsed;
    info[kBurnMediaType] = int(data.opticalMediaType);
    info[kBurnWriteSpeed] = data.opticalWriteSpeed;

    Application::dataPersistence()->setValue(kBurnAttribute, tag, info);
    Application::dataPersistence()->sync();
}

void DeviceControllerHelper::readOpticalProperty(const QString &device, BlockDeviceData *data)
{
    Q_ASSERT(device.startsWith("/dev"));
    Q_ASSERT(data);

    Application::dataPersistence()->reload();
    QString tag { device.mid(5) };

    if (Application::dataPersistence()->keys(kBurnAttribute).contains(tag)) {
        const QMap<QString, QVariant> &info = Application::dataPersistence()->value(kBurnAttribute, tag).toMap();
        data->common.sizeTotal = static_cast<qint64>(info.value(kBurnTotalSize).toLongLong());
        data->common.sizeUsed = static_cast<qint64>(info.value(kBurnUsedSize).toLongLong());
        data->opticalMediaType = static_cast<DFMBURN::MediaType>(info.value(kBurnMediaType).toInt());
        data->opticalWriteSpeed = info.value(kBurnWriteSpeed).toStringList();
    }
}
