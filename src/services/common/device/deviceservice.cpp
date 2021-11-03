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
#include "deviceservice.h"
#include "defendercontroller.h"
#include "devicemonitorhandler.h"

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/abstractfileinfo.h"

#include <dfm-mount/dfmblockmonitor.h>
#include <QtConcurrent>
#include <DDesktopServices>

#include <algorithm>

DSC_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DeviceMonitorHandler::DeviceMonitorHandler(DeviceService *serv)
    : QObject (nullptr), service(serv)
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

        // TODO(zhangs): wait dfm-mount impl connect protocol devices signal
    }
}

/*!
 * \brief disconnect device monitor for block devices and protocol devices
 */
void DeviceMonitorHandler::stopConnect()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    // disconnect block devices signal
    auto blkMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice).objectCast<DFMMOUNT::DFMBlockMonitor>();
    if (blkMonitor) {
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveAdded, this, &DeviceMonitorHandler::onBlockDriveAdded);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveRemoved, this, &DeviceMonitorHandler::onBlockDriveRemoved);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemAdded, this, &DeviceMonitorHandler::onFilesystemAdded);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemRemoved, this, &DeviceMonitorHandler::onFilesystemRemoved);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceAdded, this, &DeviceMonitorHandler::onBlockDeviceAdded);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceRemoved, this, &DeviceMonitorHandler::onBlockDeviceRemoved);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountAdded, this, &DeviceMonitorHandler::onBlockDeviceMounted);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountRemoved, this, &DeviceMonitorHandler::onBlockDeviceUnmounted);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::propertyChanged, this, &DeviceMonitorHandler::onBlockDevicePropertyChanged);

        // TODO(zhangs): wait dfm-mount impl connect protocol devices signal
    }
}

void DeviceMonitorHandler::initBlockDevicesData()
{
    auto &&blkPtrList = DeviceServiceHelper::createAllBlockDevices();
    for (auto &&blk : blkPtrList)
        insertNewBlockDeviceData(blk);
}

void DeviceMonitorHandler::initProtocolDevicesData()
{
    // TODO(zhangs): wait dfm-mount
}

void DeviceMonitorHandler::insertNewBlockDeviceData(const DeviceServiceHelper::BlockDevPtr &ptr)
{
    QString &&id = ptr->path();

    if (id.isEmpty())
        return;

    BlockDeviceData data;
    DeviceServiceHelper::makeBlockDeviceData(ptr, &data);
    allBlkDevData.insert(id, data);
}

void DeviceMonitorHandler::insertNewProtocolDeviceData(const DeviceServiceHelper::ProtocolDevPtr &ptr)
{
    // TODO(zhangs): wait dfm-mount
}

void DeviceMonitorHandler::removeBlockDeviceData(const QString &deviceId)
{
    if (allBlkDevData.contains(deviceId))
        allBlkDevData.remove(deviceId);
}

void DeviceMonitorHandler::removeProtocolDeviceData(const QString &deviceId)
{
    if (allProtocolDevData.contains(deviceId))
        allProtocolDevData.remove(deviceId);
}

void DeviceMonitorHandler::updateDataWithOpticalInfo(BlockDeviceData *data, const QMap<dfmmount::Property, QVariant> &changes)
{
    auto &&opticalFlag = DFMMOUNT::Property::DriveOptical;
    auto &&idUsageFlag = DFMMOUNT::Property::BlockIDUsage;

    // CD inserted / ejected
    if (changes.contains(opticalFlag)) {
        data->optical = changes.value(opticalFlag).toBool();
        auto &&opticalBlankFlag = DFMMOUNT::Property::DriveOpticalBlank;
        if (changes.contains(opticalBlankFlag))
            data->opticalBlank = changes.value(opticalBlankFlag).toBool();
        // TODO(zhangs): emit a signal about optical inserted or ejected
    }

    // CD recognized / not recognized
    if (changes.contains(idUsageFlag)) {
        QString &&usage = changes.value(idUsageFlag).toString().toLower();
        if (usage.toLower() == "filesystem") {
            auto &&idTypeFlag = DFMMOUNT::Property::BlockIDType;
            data->common.filesystem = changes.value(idTypeFlag).toString();
        }
    }
}

void DeviceMonitorHandler::updateDataWithMountedInfo(BlockDeviceData *data, const QMap<dfmmount::Property, QVariant> &changes)
{
    auto &&mptFlag = DFMMOUNT::Property::FileSystemMountPoint;

    // mounted / unmounted / size
    if (changes.contains(mptFlag)) {
        QString &&mpt = changes.value(mptFlag).toString();
        if (mpt.isEmpty()) { // unmounted
            data->common.mountpoint = QString("");
            data->mountpoints.clear();
        } else { // mounted
            data->common.mountpoint = mpt;
            if (!data->mountpoints.contains(mpt))
                data->mountpoints.append(mpt);
            QStorageInfo sizeInfo(mpt);
            // cannot acquire correct strorage info in optical device
            if (sizeInfo.isValid() && !data->optical) {
                data->common.sizeUsage = data->common.sizeTotal - sizeInfo.bytesAvailable();
                data->common.sizeFree = sizeInfo.bytesAvailable();
            }
            if (data->optical) {
                // TODO(zhangs): update optical disk size info (should async)
            }
        }
    }
}

void DeviceMonitorHandler::updateDataWithOtherInfo(BlockDeviceData *data, const QMap<dfmmount::Property, QVariant> &changes)
{
    auto &&idLabelFlag = DFMMOUNT::Property::BlockIDLabel;
    auto &&hintIgnoreFlag = DFMMOUNT::Property::BlockHintIgnore;
    auto &&hintSystemFlag = DFMMOUNT::Property::BlockHintSystem;

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

    // TODO(zhangs): handle other Property...
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
    dfmbase::UniversalUtils::notifyMessage(QObject::tr("The device has been safely removed"));
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceRemoved);
}

/*!
 * \brief mount block device and open url if isAutoMountAndOpenSetting is true
 * \param dev
 */
void DeviceMonitorHandler::onBlockDeviceAdded(const QString &deviceId)
{
    qInfo() << "A block device added: " << deviceId;
    auto blkDev = DeviceServiceHelper::createBlockDevice(deviceId);
    if (blkDev.isNull()) {
        qWarning() << "Dev NULL!";
        return;
    }

    insertNewBlockDeviceData(blkDev);
    emit service->blockDevAdded(deviceId);
    // maybe reload setting ?
    if (service->isInLiveSystem() || !service->isAutoMountSetting()) {
        qWarning() << "Cancel mount, live system: " << service->isInLiveSystem()
                   << "auto mount setting: " <<  service->isAutoMountSetting();
        return;
    }

    QString &&loginState = dfmbase::UniversalUtils::userLoginState();
    if (loginState != "active") {
        qWarning() << "Cancel mount, user login state is" << loginState;
        return;
    }

    if (!service->mountBlockDevice(deviceId, {})) {
         qWarning() << "Mount device failed: " << blkDev->path() << static_cast<int>(blkDev->getLastError());
         return;
    }

    if (service->isAutoMountAndOpenSetting()) {
        if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
            QString root = dfmbase::UrlRoute::rootPath(dfmbase::SchemeTypes::ROOT);
            QString mountUrlStr {root + QFileInfo(blkDev->device()).fileName() + "." + dfmbase::SuffixInfo::BLOCK};
            QProcess::startDetached(QStringLiteral("dde-file-manager"), {mountUrlStr});
            qInfo() << "open by dde-file-manager: " << mountUrlStr;
            return;
        }
        QString &&mpt = blkDev->mountPoint();
        qInfo() << "a new device mount to: " << mpt;
        DDesktopServices::showFolder(QUrl::fromLocalFile(mpt));
    }
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
}

void DeviceMonitorHandler::onFilesystemRemoved(const QString &deviceId)
{
    qInfo() << "A block device fs remvoved: " << deviceId;
    emit service->blockDevFilesystemRemoved(deviceId);
}

void DeviceMonitorHandler::onBlockDeviceMounted(const QString &deviceId, const QString &mountPoint)
{
    qInfo() << "A block device mounted: " << deviceId;
    emit service->blockDevMounted(deviceId, mountPoint);
}

void DeviceMonitorHandler::onBlockDeviceUnmounted(const QString &deviceId)
{
    qInfo() << "A block device unmounted: " << deviceId;
    emit service->blockDevUnmounted(deviceId);
}

void DeviceMonitorHandler::onBlockDevicePropertyChanged(const QString &deviceId,
                                                        const QMap<dfmmount::Property, QVariant> &changes)
{
    if (allBlkDevData.contains(deviceId)) {
        updateDataWithOpticalInfo(&allBlkDevData[deviceId], changes);
        updateDataWithMountedInfo(&allBlkDevData[deviceId], changes);
        updateDataWithOtherInfo(&allBlkDevData[deviceId], changes);

        // TODO(zhangs): support encrypted devices(reference DFMRootFileInfo::extraProperties)
    }
}

/*!
 * \class DeviceService
 *
 * \brief DeviceService provides a series of interfaces for
 * external device operations and signals for device monitoring,
 * such as mounting, unmounting, ejecting, etc.
 */

DeviceService::DeviceService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DeviceService>()
{
    monitorHandler.reset(new DeviceMonitorHandler(this));
}

DeviceService::~DeviceService()
{
    stopMonitor();
}

/*!
 * \brief auto mount block devices and protocol devices
 * !!! Note: call once
 */
void DeviceService::startAutoMount()
{
    std::call_once(DeviceServiceHelper::autoMountOnceFlag(), [this]() {
        qInfo() << "Start auto mount";
        if (isInLiveSystem()) {
            qWarning() << "Cannot auto mount, in Live System";
            return;
        }

        if (!isAutoMountSetting()) {
            qWarning() << "Cannot auto mount, AutoMount setting is false";
            return;
        }

        QStringList &&blkList = blockDevicesIdList({{"mountable", true}});
        for (const QString &id :blkList)
            mountBlockDeviceAsync(id, {{"auth.no_user_interaction", true}});

        // TODO(zhangs): mountAllProtocolDevices

        qInfo() << "End auto mount";
    });
}

bool DeviceService::startMonitor()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    monitorHandler->startConnect();
    monitorHandler->startMaintaining();
    bool ret = manager->startMonitorWatch();
    return ret;
}

bool DeviceService::stopMonitor()
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
void DeviceService::ejectBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isEjectableBlockDevice(ptr)) {
        ptr->ejectAsync(opts, [this, deviceId] (bool ret, DFMMOUNT::DeviceError err) {
            if (!ret) {
                qWarning() << "Eject failed: " << int(err);
                dfmbase::UniversalUtils::notifyMessage(tr("The device is busy, cannot eject now"));
            } else {
                emit blockDevAsyncEjected(deviceId);
            }
        });
    }
}

bool DeviceService::ejectBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isEjectableBlockDevice(ptr))
        return ptr->eject(opts);

    return false;
}

void DeviceService::poweroffBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isCanPoweroffBlockDevice(ptr)) {
        ptr->powerOffAsync(opts, [this, deviceId] (bool ret, DFMMOUNT::DeviceError err) {
            if (!ret) {
                qWarning() << "Poweroff failed: " << int(err);
                dfmbase::UniversalUtils::notifyMessage(tr("The device is busy, cannot remove now"));
            } else {
                emit blockDevAsyncPoweroffed(deviceId);
            }
        });
    }
}

bool DeviceService::poweroffBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isCanPoweroffBlockDevice(ptr))
        return ptr->powerOff(opts);

    return false;
}

bool DeviceService::stopDefenderScanDrive(const QString &deviceId)
{
    auto &&ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    QList<QUrl> &&urls = DeviceServiceHelper::makeMountpointsForDrive(ptr->drive());

    if (!DefenderInstance.stopScanning(urls)) {
        qWarning() << "stop scanning timeout";
        return false;
    }

    return true;
}

bool DeviceService::stopDefenderScanAllDrives()
{
    QList<QUrl> &&urls = DeviceServiceHelper::makeMountpointsForAllDrive();

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

void DeviceService::detachBlockDevice(const QString &deviceId)
{
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "Cannot create ptr for" << deviceId;
        return;
    }

    if (!ptr->removable()) {
        qWarning() << "Not removable device: " << deviceId;
        return;
    }

    // A block device may have more than one partition,
    // when detach a device, you need to unmount its partitions,
    // and then poweroff
    QStringList &&idList = DeviceServiceHelper::makeAllDevicesIdForDrive(ptr->drive());
    std::for_each(idList.cbegin(), idList.cend(), [this] (const QString &id) {
        if (!unmountBlockDevice(id))
            qWarning() << "Detach " << id << " abnormal, it's cannot unmount";
    });

    if (ptr->optical())
        ejectBlockDeviceAsync(deviceId);
    else
        poweroffBlockDeviceAsync(deviceId);
}

void DeviceService::detachProtocolDevice(const QString &deviceId)
{
    // TODO(zhangs):
}

void DeviceService::detachAllMountedBlockDevices()
{
    QStringList &&list = blockDevicesIdList({{"unmountable", true}});
    for (const QString &id : list)
        detachBlockDevice(id);
}

void DeviceService::detachAllMountedProtocolDevices()
{

}

void DeviceService::mountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isMountableBlockDevice(ptr)) {
        ptr->mountAsync(opts, [this, deviceId] (bool ret, DFMMOUNT::DeviceError err) {
            if (!ret)
                qWarning() << "Mount failed: " << int(err);
            else
                emit blockDevAsyncMounted(deviceId);
        });
    }
}

bool DeviceService::mountBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isMountableBlockDevice(ptr))
        return !ptr->mount(opts).isEmpty();

    return false;
}

void DeviceService::unmountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isUnmountableBlockDevice(ptr)) {
        ptr->unmountAsync(opts, [this, deviceId] (bool ret, DFMMOUNT::DeviceError err) {
            if (!ret) {
                qWarning() << "Unmount failed: " << int(err);
                dfmbase::UniversalUtils::notifyMessage(tr("Disk is busy, cannot unmount now"));
            } else {
                emit blockDevAsyncUnmounted(deviceId);
            }
        });
    }
}

bool DeviceService::unmountBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isUnmountableBlockDevice(ptr))
        return ptr->unmount(opts);

    return false;
}

bool DeviceService::isBlockDeviceMonitorWorking() const
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

bool DeviceService::isProtolDeviceMonitorWorking() const
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
bool DeviceService::isInLiveSystem() const
{
    bool ret = false;
    static const QMap<QString, QString> &cmdline = dfmbase::FileUtils::getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live"))
        ret = true;
    return ret;
}

/*!
 * \brief check property "AutoMount" of ~/.config/deepin/dde-file-manager.json
 * \return "AutoMount" property value
 */
bool DeviceService::isAutoMountSetting() const
{
    return DeviceServiceHelper::getGsGlobal()->value("GenericAttribute", "AutoMount", false).toBool();
}

/*!
 * \brief check property "AutoMountAndOpen" of ~/.config/deepin/dde-file-manager.json
 * \return "AutoMountAndOpen" property value
 */
bool DeviceService::isAutoMountAndOpenSetting() const
{
    return DeviceServiceHelper::getGsGlobal()->value("GenericAttribute", "AutoMountAndOpen", false).toBool();
}

bool DeviceService::isDefenderScanningDrive(const QString &driveName) const
{
    QList<QUrl> urls;
    if (driveName.isNull() || driveName.isEmpty())
        urls = DeviceServiceHelper::makeMountpointsForAllDrive();
    else
        urls = DeviceServiceHelper::makeMountpointsForDrive(driveName);
    return DefenderInstance.isScanning(urls);
}

/*!
 * \brief user input a opts, then return block devices list
 * \param opts: bool unmountable     -> has mounted devices(dde-dock plugin use it)
 *              bool mountable       -> has unmounted devices
 *              bool not_ignorable   -> computer and sidebar devices
 * \return devices id list
 */
QStringList DeviceService::blockDevicesIdList(const QVariantMap &opts) const
{
    QStringList idList;

    // {"unmountable" : GLib.Variant("b", True)}
    bool needUnmountable = opts.value("unmountable").toBool();
    bool needMountable = opts.value("mountable").toBool();
    bool needNotIgnorable = opts.value("not_ignorable").toBool();

    const auto &allBlkData = monitorHandler->allBlkDevData;
    for (const auto &data : allBlkData) {
        if (needUnmountable && DeviceServiceHelper::isUnmountableBlockDevice(data)) {
            idList.append(data.common.id);
            continue;
        }

        if (needMountable && DeviceServiceHelper::isMountableBlockDevice(data)) {
            idList.append(data.common.id);
            continue;
        }

        if (needNotIgnorable && !DeviceServiceHelper::isIgnorableBlockDevice(data)) {
            idList.append(data.common.id);
            continue;
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
 * \return like this:
 * {'can_power_off': True,
 * 'crypto_backingDevice': '/',
 * 'device': '/dev/sdb1',
 * 'drive': '/org/freedesktop/UDisks2/drives/USB_SanDisk_3_2e2Gen1_01013114a9e6e689e7acef289dddb66fc5abc0ff411bea158687d2016fc3863c7f4800000000000000000000b6747ae1ff1d7800a3558107b528d85f',
 * 'ejectable': True,
 * 'filesystem': 'ntfs',
 * 'has_filesystem': True,
 * 'hint_ignore': False,
 * 'hint_system': False,
 * 'id': '/org/freedesktop/UDisks2/block_devices/sdb1',
 * 'id_label': '111',
 * 'is_encrypted': False,
 * 'media_compatibility': [],
 * 'mountpoint': '',
 * 'mountpoints': ['/media/zhangs/111'],
 * 'optical': False,
 * 'optical_blank': False,
 * 'removable': True,
 * 'size_free': 47823532032,
 * 'size_total': 55500210176,
 * 'size_usage': 7676678144}
 */
QVariantMap DeviceService::blockDeviceInfo(const QString &deviceId)
{
    QVariantMap info;
    const auto &allBlkData = monitorHandler->allBlkDevData;
    if (!allBlkData.contains(deviceId))
        return info;

    const auto &blkData = allBlkData.value(deviceId);
    DeviceServiceHelper::makeBlockDeviceMap(blkData, &info);
    return info;
}

QStringList DeviceService::protocolDevicesIdList() const
{
    // TODO: wait `dfm-mount`
    return QStringList();
}

QVariantMap DeviceService::protocolDeviceInfo(const QString &deviceId)
{
    // TODO(zhangs): build data
    return QVariantMap();
}
