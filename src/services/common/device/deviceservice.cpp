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

#include "dfm-base/utils/universalutils.h"

#include <dfm-mount/dfmblockmonitor.h>
#include <QtConcurrent>
#include <DDesktopServices>

DSC_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DeviceService::DeviceService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DeviceService>()
{

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
    return manager->startMonitorWatch();
}

/*!
 * \brief device monitor for block devices and protocol devices
 */
void DeviceService::startConnect()
{
    std::call_once(DeviceServiceHelper::connectOnceFlag(), [this]() {
        auto manager = DFMMOUNT::DFMDeviceManager::instance();

        // connect block devices signal
        auto blkMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice);
        connect(blkMonitor, &DFMMOUNT::DFMBlockMonitor::driveAdded, this, &DeviceService::onBlockDriveAdded);
        connect(blkMonitor, &DFMMOUNT::DFMBlockMonitor::driveRemoved, this, &DeviceService::onBlockDriveRemoved);
        connect(blkMonitor, &DFMMOUNT::DFMBlockMonitor::deviceAdded, this, &DeviceService::onBlockDeviceAdded);
        connect(blkMonitor, &DFMMOUNT::DFMBlockMonitor::deviceRemoved, this, &DeviceService::onBlockDeviceRemoved);
        // TODO(zhangs): wait dfm-mount give a signal `fileSystemAdded`
        // TODO(zhangs): wait dfm-mount need give a param 'dfmblockdevice' pointer
        connect(blkMonitor, &DFMMOUNT::DFMBlockMonitor::mountAdded, this, &DeviceService::onBlockDeviceMounted);
        // TODO(zhangs): wait dfm-mount change mountRemvoed
        // TODO(zhangs): wait dfm-mount change propertyChanged

        // TODO(zhangs): wait dfm-mount impl connect protocol devices signal
    });
}

bool DeviceService::stopMonitor()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    return manager->stopMonitorWatch();
}

/*!
 * \brief unmount all of block devices(async) and protocol devices(sync)
 */
void DeviceService::doUnMountAll()
{
    DeviceServiceHelper::unmountAllBlockDevices();
    DeviceServiceHelper::unmountAllProtocolDevices();
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

void DeviceService::onBlockDriveAdded()
{
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceAdded);
    emit blockDriveAdded();
}

void DeviceService::onBlockDriveRemoved()
{
    dfmbase::UniversalUtils::notifyMessage(QObject::tr("The device has been safely removed"));
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceRemoved);
    emit blockDriveRemoved();
}

/*!
 * \brief mount block device and open url if isAutoMountAndOpenSetting is true
 * \param dev
 */
void DeviceService::onBlockDeviceAdded(dfmmount::DFMDevice *dev)
{
    qInfo() << "A block device added";
    auto blkDev = qobject_cast<DFMMOUNT::DFMBlockDevice *>(dev);
    if (!blkDev) {
        qWarning() << "Dev NULL!";
        return;
    }

    // maybe reload setting ?
    if (isInLiveSystem() || !isAutoMountSetting()) {
        qWarning() << "Cancel mount, live system: " << isInLiveSystem()
                   << "auto mount setting: " <<  isAutoMountSetting();
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

    if (isAutoMountAndOpenSetting()) {
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

void DeviceService::onBlockDeviceRemoved(dfmmount::DFMDevice *dev)
{

}

void DeviceService::onBlockDeviceMounted(const QString &mountPoint)
{

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
