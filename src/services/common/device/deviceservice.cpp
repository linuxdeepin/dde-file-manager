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
    auto blkMonitor = qobject_cast<QSharedPointer<DFMMOUNT::DFMBlockMonitor>>
                      (manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice));
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
    auto blkMonitor = qobject_cast<QSharedPointer<DFMMOUNT::DFMBlockMonitor>>
                      (manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice));
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
    const auto &blkPtrList = DeviceServiceHelper::createAllBlockDevices();
    for (const auto &blk : blkPtrList)
        insertNewBlockDeviceData(blk);
}

void DeviceMonitorHandler::initProtocolDevicesData()
{
    // TODO(zhangs): wait dfm-mount
}

void DeviceMonitorHandler::insertNewBlockDeviceData(const DeviceServiceHelper::BlockDevPtr &ptr)
{
    const QString &id = ptr->path();

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
    emit service->blockDeviceAdded(deviceId);
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

    if (!DeviceServiceHelper::mountBlockDevice(blkDev, {})) {
         qWarning() << "Mount device failed: " << blkDev->path() << static_cast<int>(blkDev->getLastError());
         return;
    }

    if (service->isAutoMountAndOpenSetting()) {
        if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
            // TODO(zhangs): make mount url string
            QString mountUrlStr /*= DFMROOT_ROOT + QFileInfo(blkDev->device()).fileName() + "." SUFFIX_UDISKS*/;
            QProcess::startDetached(QStringLiteral("dde-file-manager"), {mountUrlStr});
            qInfo() << "open by dde-file-manager: " << mountUrlStr;
            return;
        }
        QString &&mpt = blkDev->mountPoint().toLocalFile();
        qInfo() << "a new device mount to: " << mpt;
        DDesktopServices::showFolder(QUrl::fromLocalFile(mpt));
    }
}

void DeviceMonitorHandler::onBlockDeviceRemoved(const QString &deviceId)
{
    qInfo() << "A block device removed: " << deviceId;
    removeBlockDeviceData(deviceId);
    emit service->blockDeviceRemoved(deviceId);
}

void DeviceMonitorHandler::onFilesystemAdded(const QString &deviceId)
{
    qInfo() << "A block device fs added: " << deviceId;
    emit service->blockDeviceFilesystemAdded(deviceId);
}

void DeviceMonitorHandler::onFilesystemRemoved(const QString &deviceId)
{
    qInfo() << "A block device fs remvoved: " << deviceId;
    emit service->blockDeviceFilesystemRemoved(deviceId);
}

void DeviceMonitorHandler::onBlockDeviceMounted(const QString &deviceId, const QString &mountPoint)
{
    qInfo() << "A block device mounted: " << deviceId;
    emit service->blockDeviceMounted(deviceId, mountPoint);
}

void DeviceMonitorHandler::onBlockDeviceUnmounted(const QString &deviceId)
{
    qInfo() << "A block device unmounted: " << deviceId;
    emit service->blockDeviceUnmounted(deviceId);
}

void DeviceMonitorHandler::onBlockDevicePropertyChanged(const QString &deviceId,
                                                        const QMap<dfmmount::Property, QVariant> &changes)
{
    if (allBlkDevData.contains(deviceId)) {
        auto &&idUsageFlag = DFMMOUNT::Property::BlockIDUsage;
        auto &&mptFlag = DFMMOUNT::Property::FileSystemMountPoint;
        auto &&idLabelFlag = DFMMOUNT::Property::BlockIDLabel;
        auto &&hintIgnoreFlag = DFMMOUNT::Property::BlockHintIgnore;
        auto &curDevice = allBlkDevData[deviceId];

        // file system
        if (changes.contains(idUsageFlag)) {
            const QString &usage = changes.value(idUsageFlag).toString().toLower();
            if (usage == "filesystem") {
                const QString &fs = changes.value(DFMMOUNT::Property::BlockIDType).toString();
                curDevice.common.fileSystem = fs;
            }
        }

        // mounted / unmounted
        if (changes.contains(mptFlag)) {
            const QString &mpt = changes.value(mptFlag).toString();
            if (mpt.isEmpty()) {
                curDevice.common.mountpoint = QString("");
                curDevice.mountpoints.clear();
            } else {
                curDevice.common.mountpoint = mpt;
                curDevice.mountpoints.append(mpt);
            }
        }

        // idlable
        if (changes.contains(idLabelFlag)) {
            const QString &idlabel = changes.value(idLabelFlag).toString();
            curDevice.idLabel = idlabel;
        }

        // hintIgnore
        if (changes.contains(hintIgnoreFlag)) {
            bool hintIgnore = changes.value(hintIgnoreFlag).toBool();
            curDevice.hintIgnore = hintIgnore;
        }

        // TODO(zhangs): handle other Property...
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

        DeviceServiceHelper::mountAllBlockDevices();
        DeviceServiceHelper::mountAllProtocolDevices();
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

void DeviceService::doEject(const QString &deviceId)
{
    if (deviceId.isEmpty())
        return;

    DeviceServiceHelper::BlockDevPtr ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (ptr.isNull()) {
        qWarning() << "Cannot create block device ptr by " << deviceId;
        return;
    }

    DeviceServiceHelper::ejectBlockDevice(ptr);
}

/*!
 * \brief eject all of block devices(async) and protocol devices(sync)
 */
void DeviceService::doEjectAllMountedDevices()
{
    DeviceServiceHelper::ejectAllMountedBlockDevices();
    DeviceServiceHelper::ejectAllMountedProtocolDevices();
}

bool DeviceService::stopDefenderScanAllDrives()
{
    const QList<QUrl> &urls = DeviceServiceHelper::getMountPathForAllDrive();

    if (!DefenderInstance.stopScanning(urls)) {
        qWarning() << "stop scanning timeout";
        return false;
    }

    return true;
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
        urls = DeviceServiceHelper::getMountPathForAllDrive();
    else
        urls = DeviceServiceHelper::getMountPathForDrive(driveName);
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

QStringList DeviceService::protocolDevicesIdList() const
{
    // TODO: wait `dfm-mount`
    return QStringList();
}
