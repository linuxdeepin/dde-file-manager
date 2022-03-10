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
#include "devicecontroller.h"
#include "private/defendercontroller.h"
#include "private/devicemonitorhandler.h"

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h"

#include <dfm-mount/base/dfmmountutils.h>
#include <dfm-mount/dfmblockmonitor.h>
#include <dfm-burn/opticaldiscmanager.h>
#include <dfm-burn/opticaldiscinfo.h>

#include <QtConcurrent>
#include <DDesktopServices>

#include <algorithm>
#include <functional>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

using namespace GlobalServerDefines;

DeviceMonitorHandler::DeviceMonitorHandler(DeviceController *serv)
    : QObject(nullptr), service(serv)
{
}

/*!
 * \brief maintaining devices data
 */
void DeviceMonitorHandler::startMaintaining()
{
    initBlockDevicesData();
    initProtocolDevicesData();
}

/*!
 * \brief device monitor for block devices and protocol devices
 */
void DeviceMonitorHandler::startConnect()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    // connect block devices signal
    auto blkMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice).objectCast<DFMMOUNT::DFMBlockMonitor>();
    if (blkMonitor) {
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveAdded, this, &DeviceMonitorHandler::onBlockDriveAdded);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveRemoved, this, &DeviceMonitorHandler::onBlockDriveRemoved);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemAdded, this, &DeviceMonitorHandler::onFilesystemAdded);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemRemoved, this, &DeviceMonitorHandler::onFilesystemRemoved);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceAdded, this, &DeviceMonitorHandler::onBlockDeviceAdded);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceRemoved, this, &DeviceMonitorHandler::onBlockDeviceRemoved);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountAdded, this, &DeviceMonitorHandler::onBlockDeviceMounted);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountRemoved, this, &DeviceMonitorHandler::onBlockDeviceUnmounted);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::propertyChanged, this, &DeviceMonitorHandler::onBlockDevicePropertyChanged);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::blockUnlocked, this, &DeviceMonitorHandler::onBlockDeviceUnlocked);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::blockLocked, this, &DeviceMonitorHandler::onBlockDeviceLocked);
    }

    auto protoMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::ProtocolDevice).objectCast<DFMMOUNT::DFMProtocolMonitor>();
    if (protoMonitor) {
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::deviceAdded, this, &DeviceMonitorHandler::onProtocolDeviceAdded);
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::deviceRemoved, this, &DeviceMonitorHandler::onProtocolDeviceRemoved);
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::mountAdded, this, &DeviceMonitorHandler::onProtocolDeviceMounted);
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::mountRemoved, this, &DeviceMonitorHandler::onProtocolDeviceUnmounted);
    }

    // connect device size update worker
    connect(&sizeUpdateTimer, &QTimer::timeout, this, &DeviceMonitorHandler::onDeviceSizeUsedTimeout);
    sizeUpdateTimer.setInterval(kSizeUpdateInterval);
    sizeUpdateTimer.start();
}

/*!
 * \brief disconnect device monitor for block devices and protocol devices
 */
void DeviceMonitorHandler::stopConnect()
{
    DFM_MOUNT_USE_NS
    auto manager = DFMDeviceManager::instance();

    // disconnect block devices signal
    auto blkMonitor = manager->getRegisteredMonitor(DeviceType::BlockDevice).objectCast<DFMBlockMonitor>();
    if (blkMonitor) {
        disconnect(blkMonitor.data(), &DFMBlockMonitor::driveAdded, this, &DeviceMonitorHandler::onBlockDriveAdded);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::driveRemoved, this, &DeviceMonitorHandler::onBlockDriveRemoved);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::fileSystemAdded, this, &DeviceMonitorHandler::onFilesystemAdded);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::fileSystemRemoved, this, &DeviceMonitorHandler::onFilesystemRemoved);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::deviceAdded, this, &DeviceMonitorHandler::onBlockDeviceAdded);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::deviceRemoved, this, &DeviceMonitorHandler::onBlockDeviceRemoved);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::mountAdded, this, &DeviceMonitorHandler::onBlockDeviceMounted);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::mountRemoved, this, &DeviceMonitorHandler::onBlockDeviceUnmounted);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::propertyChanged, this, &DeviceMonitorHandler::onBlockDevicePropertyChanged);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::blockUnlocked, this, &DeviceMonitorHandler::onBlockDeviceUnlocked);
        disconnect(blkMonitor.data(), &DFMBlockMonitor::blockLocked, this, &DeviceMonitorHandler::onBlockDeviceLocked);
    }

    auto protoMonitor = manager->getRegisteredMonitor(DeviceType::ProtocolDevice).objectCast<DFMProtocolMonitor>();
    if (protoMonitor) {
        disconnect(protoMonitor.data(), &DFMProtocolMonitor::deviceAdded, this, &DeviceMonitorHandler::onProtocolDeviceAdded);
        disconnect(protoMonitor.data(), &DFMProtocolMonitor::deviceRemoved, this, &DeviceMonitorHandler::onProtocolDeviceRemoved);
        disconnect(protoMonitor.data(), &DFMProtocolMonitor::mountAdded, this, &DeviceMonitorHandler::onProtocolDeviceMounted);
        disconnect(protoMonitor.data(), &DFMProtocolMonitor::mountRemoved, this, &DeviceMonitorHandler::onProtocolDeviceUnmounted);
    }
}

void DeviceMonitorHandler::initBlockDevicesData()
{
    auto &&blkPtrList = DeviceControllerHelper::createAllBlockDevices();
    for (auto &&blk : blkPtrList)
        insertNewBlockDeviceData(blk);
}

void DeviceMonitorHandler::initProtocolDevicesData()
{
    auto &&protoPtrList = DeviceControllerHelper::createAllProtocolDevices();
    for (auto &&dev : protoPtrList)
        insertNewProtocolDeviceData(dev);
}

bool DeviceMonitorHandler::insertNewBlockDeviceData(const DeviceControllerHelper::BlockDevPtr &ptr)
{
    QString &&id = ptr->path();

    if (id.isEmpty())
        return false;

    BlockDeviceData data;
    DeviceControllerHelper::makeBlockDeviceData(ptr, &data);
    QMutexLocker guard(&mutexForBlock);
    allBlockDevData.insert(id, data);
    return true;
}

bool DeviceMonitorHandler::insertNewProtocolDeviceData(const DeviceControllerHelper::ProtocolDevPtr &ptr)
{
    auto &&id = ptr->path();
    if (id.isEmpty())
        return false;

    ProtocolDeviceData data;
    DeviceControllerHelper::makeProtocolDeviceData(ptr, &data);
    QMutexLocker guard(&mutexForProtocol);
    allProtocolDevData.insert(id, data);
    return true;
}

void DeviceMonitorHandler::removeBlockDeviceData(const QString &deviceId)
{
    QMutexLocker guard(&mutexForBlock);
    if (allBlockDevData.contains(deviceId))
        allBlockDevData.remove(deviceId);
}

void DeviceMonitorHandler::removeProtocolDeviceData(const QString &deviceId)
{
    QMutexLocker guard(&mutexForProtocol);
    if (allProtocolDevData.contains(deviceId))
        allProtocolDevData.remove(deviceId);
}

void DeviceMonitorHandler::updateDataWithOpticalInfo(BlockDeviceData *data, const QMap<DFMMOUNT::Property, QVariant> &changes)
{
    auto &&opticalFlag = DFMMOUNT::Property::DriveOptical;
    if (data->opticalDrive && changes.contains(opticalFlag)) {
        if (changes.value(opticalFlag).toBool()) {   // disc inserted
            data->optical = changes.value(opticalFlag).toBool();

            auto &&driveMediaFlag = DFMMOUNT::Property::DriveMedia;
            if (changes.contains(driveMediaFlag))
                data->media = changes.value(driveMediaFlag).toString();

            auto &&driveMediaAvailable = DFMMOUNT::Property::DriveMediaAvailable;
            if (changes.contains(driveMediaAvailable))
                data->mediaAvailable = changes.value(driveMediaAvailable).toBool();

            auto &&partitionSizeFlag = DFMMOUNT::Property::PartitionSize;
            if (changes.contains(partitionSizeFlag)) {
                data->common.sizeTotal = changes.value(partitionSizeFlag).toLongLong();
                data->common.sizeFree = 0;
                data->common.sizeUsed = data->common.sizeTotal;
            }

            auto &&driveOpticalBlank = DFMMOUNT::Property::DriveOpticalBlank;
            if (changes.contains(driveOpticalBlank))
                data->opticalBlank = changes.value(driveOpticalBlank).toBool();
        } else {   // disc ejected, clear all property associated with optical
            data->common.filesystem.clear();
            data->common.sizeUsed = 0;
            data->common.sizeFree = 0;
            data->common.sizeTotal = 0;
            data->common.mountpoint.clear();

            data->mountpoints.clear();
            data->fsVersion.clear();
            data->hasFileSystem = false;
            data->hasPartition = false;
            data->idLabel.clear();
            data->media.clear();
            data->mediaAvailable = false;
            data->optical = false;
            data->opticalBlank = false;
            data->uuid.clear();
        }
    }
}

void DeviceMonitorHandler::updateDataWithMountedInfo(BlockDeviceData *data, const QMap<DFMMOUNT::Property, QVariant> &changes)
{
    auto &&idTypeFlag = DFMMOUNT::Property::BlockIDType;
    if (changes.contains(idTypeFlag)) {
        data->common.filesystem = changes.value(idTypeFlag).toString();
        data->hasFileSystem = !(data->common.filesystem.isEmpty());
    }

    //    auto &&idUsageFlag = DFMMOUNT::Property::BlockIDUsage;
    //    if (changes.contains(idUsageFlag))

    auto &&idUUIDFlag = DFMMOUNT::Property::BlockIDUUID;
    if (changes.contains(idUUIDFlag))
        data->uuid = changes.value(idUUIDFlag).toString();

    auto &&idVersionFlag = DFMMOUNT::Property::BlockIDVersion;
    if (changes.contains(idVersionFlag))
        data->fsVersion = changes.value(idVersionFlag).toString();

    auto &&mptFlag = DFMMOUNT::Property::FileSystemMountPoint;
    if (changes.contains(mptFlag)) {
        data->mountpoints = changes.value(mptFlag).toStringList();
        if (data->mountpoints.isEmpty())
            data->common.mountpoint.clear();
        else {
            data->common.mountpoint = data->mountpoints.first();

            if (!data->opticalDrive) {
                QStorageInfo sizeInfo(data->common.mountpoint);
                data->common.sizeUsed = data->common.sizeTotal - sizeInfo.bytesAvailable();
                data->common.sizeFree = sizeInfo.bytesAvailable();
            }
        }
    }
}

void DeviceMonitorHandler::updateDataWithOtherInfo(BlockDeviceData *data, const QMap<DFMMOUNT::Property, QVariant> &changes)
{
    auto &&idLabelFlag = DFMMOUNT::Property::BlockIDLabel;
    auto &&hintIgnoreFlag = DFMMOUNT::Property::BlockHintIgnore;
    auto &&hintSystemFlag = DFMMOUNT::Property::BlockHintSystem;
    auto &&clearTextFalg = DFMMOUNT::Property::EncryptedCleartextDevice;

    // idlable
    if (changes.contains(idLabelFlag)) {
        QString &&idlabel = changes.value(idLabelFlag).toString();
        data->idLabel = idlabel;
    }

    // hintIgnore
    if (changes.contains(hintIgnoreFlag))
        data->hintIgnore = changes.value(hintIgnoreFlag).toBool();

    // hintSystem
    if (changes.contains(hintSystemFlag))
        data->hintSystem = changes.value(hintSystemFlag).toBool();

    // clearText
    if (changes.contains(clearTextFalg))
        data->cleartextDevice = changes.value(clearTextFalg).toString();
}

void DeviceMonitorHandler::handleBlockDevicesSizeUsedChanged()
{
    qDebug() << "Start check block devices size used changed";
    QList<BlockDeviceData> changedDataGroup;
    QMutexLocker guard(&mutexForBlock);
    auto &&keys = allBlockDevData.keys();
    for (const auto &key : keys) {
        auto &val = allBlockDevData[key];
        if (!DeviceControllerHelper::isIgnorableBlockDevice(val) || val.cryptoBackingDevice.length() > 1) {   // need to report the size change of unlocked device
            if (val.optical)
                continue;
            if (val.mountpoints.isEmpty())
                continue;

            const QString &id = val.common.id;
            const QString &mpt = val.common.mountpoint;
            qint64 sizeUsed = val.common.sizeUsed;

            QStorageInfo info(mpt);
            qint64 curSizeUsed = val.common.sizeTotal - info.bytesAvailable();

            if (curSizeUsed != sizeUsed) {
                qInfo() << "Block:" << id << "old size: " << sizeUsed << "new size: " << curSizeUsed;
                DeviceControllerHelper::updateBlockDeviceSizeUsed(&val, val.common.sizeTotal, info.bytesAvailable());
                changedDataGroup.push_back(val);
            }
        }
    }
    guard.unlock();

    for (auto iter = changedDataGroup.cbegin(); iter != changedDataGroup.cend(); ++iter) {
        if (Q_LIKELY(!iter->common.id.isEmpty()))
            emit service->deviceSizeUsedChanged(iter->common.id, iter->common.sizeTotal, iter->common.sizeFree);
    }
}

/*!
 * \brief DeviceMonitorHandler::handleProtolDevicesSizeUsedChanged run in thread
 */
void DeviceMonitorHandler::handleProtolDevicesSizeUsedChanged()
{
    // TODO(xust): this function is run in thread, not sure the gvfs returns correct datas in thread...
    qDebug() << "Start check protocol devices size used changed";
    QList<ProtocolDeviceData> changedDataGroup;
    QMutexLocker guard(&mutexForProtocol);
    auto &&keys = allProtocolDevData.keys();
    for (const auto &key : keys) {
        auto dev = DeviceControllerHelper::createProtocolDevice(key);
        if (dev) {
            qint64 total = dev->sizeTotal();
            qint64 free = dev->sizeFree();
            qint64 usage = dev->sizeUsage();

            auto &devData = allProtocolDevData[key];
            const qint64 &oldTotal = devData.common.sizeTotal;
            const qint64 &oldFree = devData.common.sizeFree;
            const qint64 &oldUsage = devData.common.sizeUsed;

            if (total != oldTotal
                || free != oldFree
                || usage != oldUsage) {
                qInfo() << "Protocol[new/old]: " << key
                        << QString("total: %1/%2, ").arg(total).arg(oldTotal)
                        << QString("usage: %1/%2, ").arg(usage).arg(oldUsage)
                        << QString("free: %1/%2, ").arg(free).arg(oldFree);
                DeviceControllerHelper::updateProtocolDeviceSizeUsed(&devData, total, free, usage);
                changedDataGroup.push_back(devData);
            }
        }
    }
    guard.unlock();

    for (auto iter = changedDataGroup.cbegin(); iter != changedDataGroup.cend(); ++iter) {
        if (Q_LIKELY(!iter->common.id.isEmpty()))
            emit service->deviceSizeUsedChanged(iter->common.id, iter->common.sizeTotal, iter->common.sizeFree);
    }
}

void DeviceMonitorHandler::handleOpticalDeviceChanged(const QString &deviceId)
{
    QMutexLocker guard(&mutexForBlock);
    bool devChanged { false };
    auto block = DeviceControllerHelper::createBlockDevice(deviceId);

    // Note: allBlockDevData is empty if D-Bus not used
    BlockDeviceData data;
    DeviceControllerHelper::makeBlockDeviceData(block, &data);

    QString dev { data.device };
    QScopedPointer<DFMBURN::OpticalDiscInfo> info { DFMBURN::OpticalDiscManager::createOpticalInfo(dev) };
    if (info && !data.common.id.isEmpty()) {
        devChanged = true;
        data.common.sizeTotal = static_cast<qint64>(info->totalSize());
        data.common.sizeUsed = static_cast<qint64>(info->usedSize());
        data.common.sizeFree = data.common.sizeTotal - data.common.sizeUsed;
        data.opticalMediaType = info->mediaType();
        data.opticalWriteSpeed = info->writeSpeed();
        allBlockDevData[deviceId] = data;
    }
    guard.unlock();
    if (devChanged) {
        DeviceControllerHelper::writeOpticalProperty(data);
        emit service->deviceSizeUsedChanged(data.common.id, data.common.sizeTotal, data.common.sizeFree);
    }
}

void DeviceMonitorHandler::notifyDeviceSizeUsedChanged(const QString &deviceId, const QString &mountPoint)
{
    // fix: if launch dfm without dbus service registered, the size cannot be updated in time when the device first mounted.
    QtConcurrent::run([=] {
        QMutexLocker guard(&mutexForBlock);
        auto &&keys = allBlockDevData.keys();
        if (keys.contains(deviceId)) {
            auto &oldData = allBlockDevData[deviceId];
            qlonglong sizeTotal { oldData.common.sizeTotal };
            bool optical { oldData.optical };
            QString device { oldData.device };

            qlonglong sizeFree {};
            if (optical) {
                qint64 used { 0 };
                BlockDeviceData data;
                DeviceControllerHelper::readOpticalProperty(device, &data);
                sizeTotal = data.common.sizeTotal;
                used = data.common.sizeUsed;
                sizeFree = sizeTotal - used;

                oldData.common.sizeTotal = sizeTotal;
                oldData.common.sizeUsed = used;
                oldData.common.sizeFree = sizeFree;
                oldData.opticalMediaType = data.opticalMediaType;
                oldData.opticalWriteSpeed = data.opticalWriteSpeed;
            } else {
                QStorageInfo info(mountPoint);
                sizeFree = info.bytesAvailable();
            }
            guard.unlock();

            emit service->deviceSizeUsedChanged(deviceId, sizeTotal, sizeFree);
        }
    });
}

void DeviceMonitorHandler::onBlockDriveAdded(const QString &drvObjPath)
{
    qInfo() << "A block dirve added: " << drvObjPath;
    emit service->blockDriveAdded();
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceAdded);
}

void DeviceMonitorHandler::onBlockDriveRemoved(const QString &drvObjPath)
{
    qInfo() << "A block dirve removed: " << drvObjPath;
    emit service->blockDriveRemoved();
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceRemoved);
}

/*!
 * \brief mount block device and open url if isAutoMountAndOpenSetting is true
 * \param dev
 */
void DeviceMonitorHandler::onBlockDeviceAdded(const QString &deviceId)
{
    qInfo() << "A block device added: " << deviceId;
    auto blkDev = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!blkDev) {
        qWarning() << "Dev NULL!";
        return;
    }

    if (!insertNewBlockDeviceData(blkDev))
        return;

    emit service->blockDevAdded(deviceId);
    // maybe reload setting ?
    if (service->isInLiveSystem() || !service->isAutoMountSetting()) {
        qWarning() << "Cancel mount, live system: " << service->isInLiveSystem()
                   << "auto mount setting: " << service->isAutoMountSetting();
        return;
    }

    QString &&loginState = DFMBASE_NAMESPACE::UniversalUtils::userLoginState();
    if (loginState != "active") {
        qWarning() << "Cancel mount, user login state is" << loginState;
        return;
    }

    bool isUnlockedDevice = blkDev->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString().length() > 1;
    if (isUnlockedDevice) {
        qDebug() << "No auto mount for unlocked device: " << blkDev->path();
        return;
    }

    if (service->mountBlockDevice(deviceId, {}).isEmpty()) {
        qWarning() << "Mount device failed: " << blkDev->path();
        return;
    }

    if (service->isAutoMountAndOpenSetting())
        DeviceControllerHelper::openFileManagerToDevice(blkDev);
}

void DeviceMonitorHandler::onBlockDeviceRemoved(const QString &deviceId)
{
    qInfo() << "A block device removed: " << deviceId;
    removeBlockDeviceData(deviceId);
    emit service->blockDevRemoved(deviceId);
}

void DeviceMonitorHandler::onFilesystemAdded(const QString &deviceId)
{
    qInfo() << "A block device fs added: " << deviceId;
    emit service->blockDevFilesystemAdded(deviceId);
    emit service->blockDevicePropertyChanged(deviceId, DeviceProperty::kHasFileSystem, true);
}

void DeviceMonitorHandler::onFilesystemRemoved(const QString &deviceId)
{
    qInfo() << "A block device fs removed: " << deviceId;
    emit service->blockDevFilesystemRemoved(deviceId);
    emit service->blockDevicePropertyChanged(deviceId, DeviceProperty::kHasFileSystem, false);
}

void DeviceMonitorHandler::onBlockDeviceMounted(const QString &deviceId, const QString &mountPoint)
{
    qInfo() << "A block device mounted: " << deviceId;
    emit service->blockDevMounted(deviceId, mountPoint);

    notifyDeviceSizeUsedChanged(deviceId, mountPoint);
}

void DeviceMonitorHandler::onBlockDeviceUnmounted(const QString &deviceId)
{
    qInfo() << "A block device unmounted: " << deviceId;
    emit service->blockDevUnmounted(deviceId);
}

void DeviceMonitorHandler::onBlockDevicePropertyChanged(const QString &deviceId,
                                                        const QMap<DFMMOUNT::Property, QVariant> &changes)
{
    QMutexLocker guard(&mutexForBlock);
    if (allBlockDevData.contains(deviceId)) {
        updateDataWithOpticalInfo(&allBlockDevData[deviceId], changes);
        updateDataWithMountedInfo(&allBlockDevData[deviceId], changes);
        updateDataWithOtherInfo(&allBlockDevData[deviceId], changes);
    }
    guard.unlock();
    QList<DFMMOUNT::Property> &&keys = changes.keys();
    for (DFMMOUNT::Property k : keys) {
        QString propertyName { DFMMOUNT::Utils::getNameByProperty(k) };
        if (Q_LIKELY(!propertyName.isEmpty())) {
            emit service->blockDevicePropertyChanged(deviceId, propertyName, changes.value(k));
            qInfo() << "Block Device: " << deviceId << "property: " << propertyName
                    << "changed!"
                    << "New value is:" << changes.value(k);
        }
    }
}

void DeviceMonitorHandler::onBlockDeviceUnlocked(const QString &deviceId, const QString &clearDeviceId)
{
    emit service->blockDevUnlocked(deviceId, clearDeviceId);
}

void DeviceMonitorHandler::onBlockDeviceLocked(const QString &deviceId)
{
    emit service->blockDevLocked(deviceId);
}

void DeviceMonitorHandler::onDeviceSizeUsedTimeout()
{
    QtConcurrent::run([this]() {
        handleBlockDevicesSizeUsedChanged();
        handleProtolDevicesSizeUsedChanged();
    });
}

void DeviceMonitorHandler::onProtocolDeviceAdded(const QString &deviceId)
{
    auto protoDev = DeviceControllerHelper::createProtocolDevice(deviceId);
    if (!protoDev) {
        qWarning() << "protocol device is null!!";
        return;
    }
    qInfo() << "A new protocol device is added: " << protoDev->displayName() << deviceId;

    if (!insertNewProtocolDeviceData(protoDev))
        return;

    emit service->protocolDevAdded(deviceId);

    if (service->isInLiveSystem() || !service->isAutoMountSetting()) {
        qWarning() << "Cancel mount, live system: " << service->isInLiveSystem()
                   << "auto mount setting: " << service->isAutoMountSetting();
        return;
    }

    QString &&loginState = DFMBASE_NAMESPACE::UniversalUtils::userLoginState();
    if (loginState != "active") {
        qWarning() << "Cancel mount, user login state is" << loginState;
        return;
    }

    // TODO(xust) do mount volume here
    if (service->mountProtocolDevice(deviceId, {}).isEmpty()) {
        qWarning() << "Mount device failed: " << protoDev->displayName();
        return;
    }
}

void DeviceMonitorHandler::onProtocolDeviceRemoved(const QString &deviceId)
{
    qInfo() << "A new protocol device is removed: " << deviceId;
    removeProtocolDeviceData(deviceId);

    emit service->protocolDevRemoved(deviceId);
}

void DeviceMonitorHandler::onProtocolDeviceMounted(const QString &deviceId, const QString &mountPoint)
{
    qInfo() << "A protocol device is mounted at: " << mountPoint;
    auto protoDev = DeviceControllerHelper::createProtocolDevice(deviceId);
    if (!protoDev) {
        qWarning() << deviceId << "constructed a null device!!!";
        return;
    }

    // if it already existed in cache, then update it.
    insertNewProtocolDeviceData(protoDev);
    emit service->protocolDevMounted(deviceId, mountPoint);
}

void DeviceMonitorHandler::onProtocolDeviceUnmounted(const QString &deviceId)
{
    qInfo() << "A protocol device in unmounted: " << deviceId;
    auto protoDev = DeviceControllerHelper::createProtocolDevice(deviceId);
    if (!protoDev) {
        removeProtocolDeviceData(deviceId);
    } else {
        insertNewProtocolDeviceData(protoDev);
    }

    emit service->protocolDevUnmounted(deviceId);
}

/*!
 * \class DeviceService
 *
 * \brief DeviceService provides a series of interfaces for
 * external device operations and signals for device monitoring,
 * such as mounting, unmounting, ejecting, etc.
 */

DeviceController::DeviceController(QObject *parent)
    : QObject(parent)
{
    monitorHandler.reset(new DeviceMonitorHandler(this));
}

DeviceController::~DeviceController()
{
    //    stopMonitor();   // DFMDeviceMonitor is released before this function invoked.
}

/*!
 * \brief DeviceService::askForStopScanning, before unmount device, check if need to stop scanning
 * \param deviceId
 * \return
 */
bool DeviceController::askForStopScanning(const QString &deviceId)
{
    if (isDefenderScanningDrive(deviceId)) {
        DDialog *dlg = DialogManagerInstance->showQueryScanningDialog(tr("Scanning the device, stop it?"));
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        if (dlg->exec() == QDialog::Accepted) {
            if (!stopDefenderScanDrive(deviceId)) {
                qWarning() << "stop scanning device failed: " << deviceId;
                DialogManagerInstance->showErrorDialog(tr("Unmount failed"), tr("Cannot stop scanning device"));
                return false;
            } else {
                return true;
            }
        } else {
            return false;
        }
    }
    return true;
}

DeviceController *DeviceController::instance()
{
    static DeviceController controller;
    return &controller;
}

/*!
 * \brief auto mount block devices and protocol devices
 * !!! Note: call once
 */
void DeviceController::startAutoMount()
{
    std::call_once(DeviceControllerHelper::autoMountOnceFlag(), [this]() {
        qInfo() << "Start auto mount";
        if (isInLiveSystem()) {
            qWarning() << "Cannot auto mount, in Live System";
            return;
        }

        if (!isAutoMountSetting()) {
            qWarning() << "Cannot auto mount, AutoMount setting is false";
            return;
        }

        QStringList &&blkList = blockDevicesIdList({ { ListOpt::kMountable, true } });
        for (const QString &id : blkList)
            mountBlockDeviceAsync(id, { { "auth.no_user_interaction", true } });

        QStringList &&protocolList = protocolDevicesIdList();
        for (const QString &id : protocolList)
            mountProtocolDeviceAsync(id, {});

        qInfo() << "End auto mount";
    });
}

bool DeviceController::startMonitor()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    monitorHandler->startConnect();
    monitorHandler->startMaintaining();
    bool ret = manager->startMonitorWatch();
    return ret;
}

bool DeviceController::stopMonitor()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    monitorHandler->stopConnect();
    bool ret = manager->stopMonitorWatch();
    return ret;
}

/*!
 * \brief async eject a block device
 * \param deviceId
 */
void DeviceController::ejectBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallback callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
        return;
    }

    if (DeviceControllerHelper::isEjectableBlockDevice(ptr)) {
        ptr->ejectAsync(opts, callback);
    } else {
        if (callback) {
            callback(false, DFMMOUNT::DeviceError::UserErrorNotEjectable);
            qWarning() << "device is not ejectable: " << deviceId;
        }
    }
}

bool DeviceController::ejectBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        return false;
    }

    if (DeviceControllerHelper::isEjectableBlockDevice(ptr))
        return ptr->eject(opts);

    qWarning() << "device is not ejectable: " << deviceId;
    return false;
}

void DeviceController::poweroffBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallback callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
        return;
    }

    if (DeviceControllerHelper::isCanPoweroffBlockDevice(ptr)) {
        ptr->powerOffAsync(opts, callback);
    } else {
        qWarning() << "device can not be poweroffed: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
    }
}

bool DeviceController::poweroffBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        return false;
    }

    if (DeviceControllerHelper::isCanPoweroffBlockDevice(ptr))
        return ptr->powerOff(opts);

    qWarning() << "device cannot be poweroffed: " << deviceId;
    return false;
}

bool DeviceController::renameBlockDevice(const QString &deviceId, const QString &newName, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        return false;
    }

    if (!ptr->mountPoints().isEmpty()) {
        qWarning() << "device cannot be renamed: " << deviceId << "cause it's mounted at" << ptr->mountPoints().first();
        return false;
    }
    return ptr->rename(newName, opts);
}

void DeviceController::renameBlockDeviceAsync(const QString &deviceId, const QString &newName, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallback callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
        return;
    }

    if (!ptr->mountPoints().isEmpty()) {
        qWarning() << "device cannot be renamed: " << deviceId << "cause it's mounted at" << ptr->mountPoints().first();
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UserErrorAlreadyMounted);
        return;
    }
    ptr->renameAsync(newName, opts, callback);
}

QString DeviceController::unlockBlockDevice(const QString &passwd, const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        return "";
    }

    if (ptr->isEncrypted()) {
        QString unlockdedObjPath;
        bool ok = ptr->unlock(passwd, unlockdedObjPath, opts);
        if (!ok) {
            qDebug() << "cannot unlock block device: " << deviceId
                     << DFMMOUNT::Utils::errorMessage(ptr->lastError());
            return "";
        } else {
            return unlockdedObjPath;
        }
    }
    qWarning() << "cannot unlock block device cause it's not encrypted: " << deviceId;
    return "";
}

void DeviceController::unlockBlockDeviceAsync(const QString &passwd, const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallbackWithMessage callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError, "");
        return;
    }

    if (ptr->isEncrypted()) {
        ptr->unlockAsync(passwd, opts, callback);
    } else {
        qWarning() << "device cannot be unlocked: " << deviceId << "cause it's not a encrypted device";
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UserErrorNotEncryptable, "");
    }
}

bool DeviceController::lockBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        return false;
    }

    if (!ptr->mountPoints().isEmpty()) {
        qWarning() << "cannot lock device cause it's mounted: " << deviceId << ptr->mountPoints();
        return false;
    }

    bool ok = ptr->lock(opts);
    if (!ok) {
        qWarning() << "cannot lock device: " << deviceId
                   << DFMMOUNT::Utils::errorMessage(ptr->lastError());
    }
    return ok;
}

void DeviceController::lockBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallback callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
        return;
    }

    if (!ptr->isEncrypted()) {
        qWarning() << "cannot lock device cause it's not encrypted: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
        return;
    }

    ptr->lockAsync(opts, callback);
}

QString DeviceController::mountProtocolDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create protocol device: " << deviceId;
        return "";
    }

    return ptr->mount(opts);
}

void DeviceController::mountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallbackWithMessage callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create protocol device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError, "");
        return;
    }

    ptr->mountAsync(opts, callback);
}

bool DeviceController::unmountProtocolDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create protocol device: " << deviceId;
        return false;
    }

    return ptr->unmount(opts);
}

void DeviceController::unmountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallback callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create protocol device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
        return;
    }

    ptr->unmountAsync(opts, callback);
}

static DFMMOUNT::MountPassInfo askForPasswdWhenMountNetworkDevice(const QString &message, const QString &userDefault, const QString &domainDefault, const QString &uri)
{
    MountAskPasswordDialog dlg;
    dlg.setTitle(message);
    dlg.setDomain(domainDefault);
    dlg.setUser(userDefault);

    if (uri.startsWith("ftp") || uri.startsWith("sftp"))
        dlg.setDomainLineVisible(false);

    DFMMOUNT::MountPassInfo info;
    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject loginInfo = dlg.getLoginData();
        auto data = loginInfo.toVariantMap();
        using namespace GlobalServerDefines::NetworkMountParamKey;
        if (data.contains(kAnonymous) && data.value(kAnonymous).toBool()) {
            info.anonymous = true;
        } else {
            info.userName = data.value(kUser).toString();
            info.domain = data.value(kDomain).toString();
            info.passwd = data.value(kPasswd).toString();
            info.savePasswd = static_cast<DFMMOUNT::NetworkMountPasswdSaveMode>(data.value(kPasswdSaveMode).toInt());
        }
    } else {
        info.cancelled = true;
    }

    return info;
}

void DeviceController::mountNetworkDevice(const QString &address, DFMMOUNT::DeviceOperateCallbackWithMessage callback)
{
    Q_ASSERT_X(!address.isEmpty(), "DeviceService", "address is emtpy");

    using namespace std::placeholders;
    auto func = std::bind(askForPasswdWhenMountNetworkDevice, _1, _2, _3, address);

    DFMMOUNT::DFMProtocolDevice::mountNetworkDevice(address, func, callback);
}

bool DeviceController::stopDefenderScanDrive(const QString &deviceId)
{
    auto &&ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    QList<QUrl> &&urls = DeviceControllerHelper::makeMountpointsForDrive(ptr->drive());

    if (!DefenderInstance.stopScanning(urls)) {
        qWarning() << "stop scanning timeout";
        return false;
    }

    return true;
}

bool DeviceController::stopDefenderScanAllDrives()
{
    QList<QUrl> &&urls = DeviceControllerHelper::makeMountpointsForAllDrive();

    if (!DefenderInstance.stopScanning(urls)) {
        qWarning() << "stop scanning timeout";
        return false;
    }

    return true;
}

/*!
 * \brief detach a block device (dde-dock plugin is eject, dde-file-manager
 * is safely remove)
 * \param deviceId is block device path
 * \param removeOptical is only effetive for optical disk
 * \return device ejected or poweroffed
 */

bool DeviceController::detachBlockDevice(const QString &deviceId)
{
    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "Cannot create ptr for" << deviceId;
        return false;
    }

    if (!ptr->removable()) {
        qWarning() << "Not removable device: " << deviceId;
        return false;
    }

    // A block device may have more than one partition,
    // when detach a device, you need to unmount its partitions,
    // and then poweroff
    bool isAllUnmounted = true;
    QStringList &&idList = DeviceControllerHelper::makeAllDevicesIdForDrive(ptr->drive());
    std::for_each(idList.cbegin(), idList.cend(), [this, &isAllUnmounted](const QString &id) {
        if (!unmountBlockDevice(id)) {
            qWarning() << "Detach " << id << " abnormal, it's cannot unmount";
            isAllUnmounted = false;
        }
    });

    if (!isAllUnmounted)
        return false;

    if (ptr->mediaCompatibility().join(" ").contains("optical")) {
        if (ptr->optical())
            ejectBlockDeviceAsync(deviceId);
    } else {
        poweroffBlockDeviceAsync(deviceId);
    }
    return true;
}

bool DeviceController::detachProtocolDevice(const QString &deviceId)
{
    // for protocol devices, there is no eject/poweroff, so just unmount them
    unmountProtocolDeviceAsync(deviceId);
    return true;
}

bool DeviceController::detachAllMountedBlockDevices()
{
    QStringList &&list = blockDevicesIdList({ { ListOpt::kUnmountable, true } });
    bool isAllDetached = true;
    for (const QString &id : list)
        isAllDetached &= detachBlockDevice(id);
    return isAllDetached;
}

bool DeviceController::detachAllMountedProtocolDevices()
{
    QStringList &&list = protocolDevicesIdList();
    bool isAllDetached = true;
    for (const QString &id : list)
        isAllDetached &= detachProtocolDevice(id);
    return isAllDetached;
}

void DeviceController::mountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallbackWithMessage callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError, "");
        return;
    }

    if (ptr->optical()) {
        QFutureWatcher<void> *fw(new QFutureWatcher<void>);
        connect(fw, &QFutureWatcher<void>::finished, this, [=]() {
            ptr->mountAsync(opts, callback);
            if (fw)
                delete fw;
        });
        fw->setFuture(QtConcurrent::run([=] {
            monitorHandler->handleOpticalDeviceChanged(deviceId);
        }));
    } else {
        QString errMsg;
        if (DeviceControllerHelper::isMountableBlockDevice(ptr, &errMsg)) {
            ptr->mountAsync(opts, callback);
        } else {
            qWarning() << "device is not mountable: " << deviceId << errMsg;
            if (callback)
                callback(false, DFMMOUNT::DeviceError::UserErrorNotMountable, "");
        }
    }
}

QString DeviceController::mountBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot craete block device: " << deviceId;
        return "";
    }

    // Note: Blocking main thread!
    if (ptr->optical())
        monitorHandler->handleOpticalDeviceChanged(deviceId);

    QString errMsg;
    if (DeviceControllerHelper::isMountableBlockDevice(ptr, &errMsg)) {
        return ptr->mount(opts);
    } else {
        // Note: Blocking main thread!
        if (ptr->optical())
            monitorHandler->handleOpticalDeviceChanged(deviceId);
        qWarning() << "Not mountable device: " << errMsg;
    }
    return "";
}

void DeviceController::unmountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts, DFMMOUNT::DeviceOperateCallback callback)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
        return;
    }

    if (!askForStopScanning(deviceId))
        return;

    QString errMsg;
    if (!DeviceControllerHelper::isUnmountableBlockDevice(ptr, &errMsg)) {
        qWarning() << "device is not unmountable: " << deviceId << errMsg;
        if (callback)
            callback(false, DFMMOUNT::DeviceError::UnhandledError);
    } else {
        ptr->unmountAsync(opts, callback);
    }
}

bool DeviceController::unmountBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");

    auto ptr = DeviceControllerHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "cannot create block device: " << deviceId;
        return false;
    }

    if (!askForStopScanning(deviceId))
        return false;

    QString errMsg;
    if (!DeviceControllerHelper::isUnmountableBlockDevice(ptr, &errMsg)) {
        qWarning() << "device is unmountable: " << deviceId << errMsg;
        return false;
    } else {
        if (ptr->isEncrypted())
            return unmountBlockDevice(ptr->getProperty(DFMMOUNT::Property::EncryptedCleartextDevice).toString(), opts)
                    && ptr->lock();
        if (!ptr->hasFileSystem())
            return true;
        if (ptr->mountPoints().isEmpty())
            return true;
        return ptr->unmount(opts);
    }
}

bool DeviceController::isBlockDeviceMonitorWorking() const
{
    bool ret = false;
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    if (manager) {
        auto blkMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice);
        if (blkMonitor && blkMonitor->status() == DFMMOUNT::MonitorStatus::Monitoring)
            ret = true;
    }

    return ret;
}

bool DeviceController::isProtolDeviceMonitorWorking() const
{
    bool ret = false;
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    if (manager) {
        auto protocolMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::ProtocolDevice);
        if (protocolMonitor && protocolMonitor->status() == DFMMOUNT::MonitorStatus::Monitoring)
            ret = true;
    }

    return ret;
}

/*!
 * \brief check if we are in live system, don't do auto mount if we are in live system
 * \return true if live system
 */
bool DeviceController::isInLiveSystem() const
{
    bool ret = false;
    static const QMap<QString, QString> &cmdline = DFMBASE_NAMESPACE::FileUtils::getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live"))
        ret = true;
    return ret;
}

/*!
 * \brief check property "AutoMount" of ~/.config/deepin/dde-file-manager.json
 * \return "AutoMount" property value
 */
bool DeviceController::isAutoMountSetting() const
{
    return DeviceControllerHelper::getGsGlobal()->value("GenericAttribute", "AutoMount", false).toBool();
}

/*!
 * \brief check property "AutoMountAndOpen" of ~/.config/deepin/dde-file-manager.json
 * \return "AutoMountAndOpen" property value
 */
bool DeviceController::isAutoMountAndOpenSetting() const
{
    return DeviceControllerHelper::getGsGlobal()->value("GenericAttribute", "AutoMountAndOpen", false).toBool();
}

bool DeviceController::isDefenderScanningDrive(const QString &deviceId) const
{
    QList<QUrl> urls;
    QString driveName;

    if (!deviceId.isEmpty()) {
        auto &&ptr = DeviceControllerHelper::createBlockDevice(deviceId);
        if (ptr)
            driveName = ptr->drive();
    }

    if (driveName.isNull() || driveName.isEmpty())
        urls = DeviceControllerHelper::makeMountpointsForAllDrive();
    else
        urls = DeviceControllerHelper::makeMountpointsForDrive(driveName);
    return DefenderInstance.isScanning(urls);
}

/*!
 * \brief user input a opts, then return block devices list
 * \param opts: bool unmountable     -> has mounted devices(dde-dock plugin use it)
 *              bool mountable       -> has unmounted devices
 *              bool not_ignorable   -> computer and sidebar devices
 * \return devices id list
 */
QStringList DeviceController::blockDevicesIdList(const QVariantMap &opts) const
{
    QStringList idList;

    // {"unmountable" : GLib.Variant("b", True)}
    bool needUnmountable = opts.value(ListOpt::kUnmountable).toBool();
    bool needMountable = opts.value(ListOpt::kMountable).toBool();
    bool needNotIgnorable = opts.value(ListOpt::kNotIgnorable).toBool();

    const auto allBlkData = monitorHandler->allBlockDevData;   // must use value!!!
    for (const auto &data : allBlkData) {
        if (needUnmountable && DeviceControllerHelper::isUnmountableBlockDevice(data)) {
            idList.append(data.common.id);
            continue;
        }

        if (needMountable && DeviceControllerHelper::isMountableBlockDevice(data)) {
            idList.append(data.common.id);
            continue;
        }

        QString errMsg;
        if (needNotIgnorable && !DeviceControllerHelper::isIgnorableBlockDevice(data, &errMsg)) {
            idList.append(data.common.id);
            continue;
        } else if (!errMsg.isEmpty()) {
            qWarning() << "Device has beeen ignore: " << errMsg;
        }

        if (!needUnmountable && !needMountable && !needNotIgnorable) {
            idList.append(data.common.id);
            continue;
        }
    }

    return idList;
}

/*!
 * \brief make a map that contains all info for the block device
 * \param deviceId
 * \param detail: return all if true
 * if input:  '/org/freedesktop/UDisks2/block_devices/sr0', GLib.Variant("b", True)
 * \return like this:
 * {'can_power_off': True,
 *  'crypto_backingDevice': '/',
 *  'device': '/dev/sr0',
 *  'drive': '/org/freedesktop/UDisks2/drives/HL_DT_ST_DVDRAM_GP75N_KXDJAVB0521',
 *  'ejectable': True,
 *  'filesystem': 'iso9660',
 *  'has_filesystem': True,
 *  'has_partition_table': False,
 *  'hint_ignore': False,
 *  'hint_system': False,
 *  'id': '/org/freedesktop/UDisks2/block_devices/sr0',
 *  'id_label': '',
 *  'is_encrypted': False,
 *  'is_loop_device': False,
 *  'media': 'optical_dvd_r',
 *  'media_available': True,
 *  'media_compatibility': ['optical_cd',
 *                          'optical_cd_r',
 *                          'optical_cd_rw',
 *                          'optical_dvd',
 *                          'optical_dvd_plus_r',
 *                          'optical_dvd_plus_r_dl',
 *                          'optical_dvd_plus_rw',
 *                          'optical_dvd_r',
 *                          'optical_dvd_ram',
 *                          'optical_dvd_rw',
 *                          'optical_mrw',
 *                          'optical_mrw_w'],
 *  'mountpoint': '/media/zhangs/2021-10-15-09-50-02-00',
 *  'mountpoints': ['/media/zhangs/2021-10-15-09-50-02-00'],
 *  'optical': True,
 *  'optical_blank': False,
 *  'removable': True,
 *  'size_free': 0,
 *  'size_total': 393216,
 *  'size_usage': 393216}
 */
QVariantMap DeviceController::blockDeviceInfo(const QString &deviceId, bool detail) const
{
    QVariantMap info;
    const auto allBlkData = monitorHandler->allBlockDevData;   // must use value!!!
    if (!allBlkData.contains(deviceId))
        return info;

    const auto &&blkData = allBlkData.value(deviceId);
    DeviceControllerHelper::makeBlockDeviceMap(blkData, &info, detail);
    return info;
}

QStringList DeviceController::protocolDevicesIdList() const
{
    QStringList idList;

    const auto allProtoData = monitorHandler->allProtocolDevData;
    auto iter = allProtoData.cbegin();
    while (iter != allProtoData.cend()) {
        idList.append(iter.value().common.id);
        iter += 1;
    }
    return idList;
}

QVariantMap DeviceController::protocolDeviceInfo(const QString &deviceId, bool detail) const
{
    QVariantMap info;
    const auto allProtoData = monitorHandler->allProtocolDevData;
    if (!allProtoData.contains(deviceId))
        return info;

    const auto &protoData = allProtoData.value(deviceId);
    DeviceControllerHelper::makeProtocolDeviceMap(protoData, &info, detail);
    return info;
}

void DeviceController::ghostBlockDevMounted(const QString &deviceId, const QString &mountPoint)
{
    monitorHandler->notifyDeviceSizeUsedChanged(deviceId, mountPoint);
}
