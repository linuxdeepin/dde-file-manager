/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "interfaces/dumountmanager.h"
#include "interfaces/defenderinterface.h"

#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "ddiskmanager.h"

#include <QUrl>
#include <QScopedPointer>
#include <QDebug>

DUMountManager::DUMountManager(QObject *parent)
    :QObject(parent)
{
    m_defenderInterface.reset(new DefenderInterface);
}

DUMountManager::~DUMountManager()
{
}

QString DUMountManager::getDriveName(const QString &blkName)
{
    QScopedPointer<DBlockDevice> blkd(DDiskManager::createBlockDevice(blkName));
    return blkd ? blkd->drive() : QString();
}

bool DUMountManager::isScanningBlock(const QString &blkName)
{
    QUrl url = getMountPathForBlock(blkName);
    return m_defenderInterface->isScanning(url);
}

bool DUMountManager::isScanningDrive(const QString &driveName)
{
    QList<QUrl> urls;
    if (driveName.isNull() || driveName.isEmpty())
        urls = getMountPathForAllDrive();
    else
        urls = getMountPathForDrive(driveName);
    return m_defenderInterface->isScanning(urls);
}

bool DUMountManager::stopScanBlock(const QString &blkName)
{
    const QUrl &url = getMountPathForBlock(blkName);
    if (!m_defenderInterface->stopScanning(url)) {
        errorMsg = "stop scanning timeout.";
        return false;
    }

    clearError();
    return true;
}

bool DUMountManager::stopScanDrive(const QString &driveName)
{
    const QList<QUrl> &urls = getMountPathForDrive(driveName);
    if (!m_defenderInterface->stopScanning(urls)) {
        errorMsg = "stop scanning timeout.";
        return false;
    }

    clearError();
    return true;
}

bool DUMountManager::stopScanAllDrive()
{
    const QList<QUrl> &urls = getMountPathForAllDrive();
    if (!m_defenderInterface->stopScanning(urls)) {
        errorMsg = "stop scanning timeout.";
        return false;
    }

    clearError();
    return true;
}

bool DUMountManager::umountBlock(const QString &blkName)
{
    QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(blkName));
    if (!blkdev) {
        errorMsg = "invalid blk device";
        qWarning() << "invalid blk device:" << blkName;
        return false;
    }

    if (blkdev->isEncrypted())
        blkdev.reset(DDiskManager::createBlockDevice(blkdev->cleartextDevice()));

    qInfo() << "umount start:" << blkName;
    if (!blkdev->mountPoints().empty())
        blkdev->unmount({});

    qInfo() << "umount done:" << blkName;

    QDBusError err = blkdev->lastError();
    if (blkdev->cryptoBackingDevice().length() > 1) {
        QScopedPointer<DBlockDevice> cbblk(DDiskManager::createBlockDevice(blkdev->cryptoBackingDevice()));
        if (cbblk) {
            cbblk->lock({});
            if (cbblk->lastError().isValid())
                err = cbblk->lastError();
        }
    }

    // 检查挂载点移除代表卸载成功
    if (blkdev->mountPoints().empty())
        return true;

    errorMsg = checkMountErrorMsg(err);
    return false;
}

bool DUMountManager::umountBlocksOnDrive(const QString &driveName)
{
    if (driveName.isNull() || driveName.isEmpty()) {
        qWarning() << "invalid drive name:" << driveName;
        errorMsg = "invalid drive name" ;
        return false;
    }

    qInfo() << "start umount blocks on drive:" << driveName;
    for (const QString &blkStr : DDiskManager::blockDevices({})) {
        QScopedPointer<DBlockDevice> blkd(DDiskManager::createBlockDevice(blkStr));
        if (blkd && blkd->drive() == driveName) {
            if (!umountBlock(blkStr)) {
                qWarning() << "umountBlock failed: drive = " << driveName << ", block str = " << blkStr;
                errorMsg = "umount block failed";
                return false;
            }
        }
    }
    return true;
}

QString DUMountManager::checkMountErrorMsg(const QDBusError &dbsError)
{
    if (!dbsError.isValid())
        return QString();

    if (dbsError.type() == QDBusError::NoReply)
        return tr("Authentication timed out");

    return tr("Disk is busy, cannot unmount now");
}

QString DUMountManager::checkEjectErrorMsg(const QDBusError &dbsError)
{
    if (!dbsError.isValid())
        return QString();

    if (dbsError.type() == QDBusError::NoReply)
        return tr("Authentication timed out");

    // 其他错误,统一提示,设备繁忙
    return tr("Disk is busy, cannot eject now");
}

bool DUMountManager::removeDrive(const QString &driveName)
{
    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(driveName));
    if (!drv) {
        errorMsg = "invalid drive.";
        return false;
    }

    // 尝试性卸载, 错误处理依赖后续流程
    umountBlocksOnDrive(driveName);

    qInfo() << "start remove drive:" << driveName;
    if (drv->canPowerOff()) {
        drv->powerOff({});

        if (drv->lastError().isValid()) {
            qWarning() << drv->lastError() << "id:" << drv->lastError().type();
            errorMsg = tr("The device is busy, cannot remove now");
        }
    }

    DDiskManager diskManager;
    QStringList devices = diskManager.diskDevices();
    qInfo() << "rest devices:" << devices;

    // 磁盘已经不在磁盘列表, 代表移除成功
    if (!devices.contains(driveName))
        return true;

    // 处理错误
    errorMsg = checkEjectErrorMsg(drv->lastError());
    return false;
}

bool DUMountManager::ejectDrive(const QString &driveName)
{
    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(driveName));
    if (!drv) {
        errorMsg = "invalid drive.";
        return false;
    }

    // 尝试性卸载, 错误处理依赖后续流程
    umountBlocksOnDrive(driveName);

    qInfo() << "start eject drive:" << driveName;
    if (drv->optical()) {
        if (drv->ejectable()) {
            drv->eject({});

            if (drv->lastError().isValid()) {
                qWarning() << drv->lastError() << "id:" << drv->lastError().type();
                errorMsg = tr("The device is busy, cannot eject now");
                return false;
            }

            qInfo() << "eject done:" << driveName;
            return true;
        }
    }

    if (drv->removable()) {
        drv->eject({});

        if (drv->lastError().isValid()) {
            qWarning() << drv->lastError() << "id:" << drv->lastError().type();
            errorMsg = tr("The device is busy, cannot remove now");
            return false;
        }
    }

    if (drv->canPowerOff()) {
        drv->powerOff({});

        if (drv->lastError().isValid()) {
            qWarning() << drv->lastError() << "id:" << drv->lastError().type();
            errorMsg = tr("The device is busy, cannot remove now");
            return false;
        }
    }

    return true;
}

bool DUMountManager::ejectAllDrive()
{
    DDiskManager diskManager;
    QStringList blockDevices = diskManager.blockDevices({});
    QSet<QString> removableDrives;

    for (const QString &blDevStr : blockDevices) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));
        if (!blDev)
            continue;

        const QString &driveName = blDev->drive();
        if (driveName.isNull() || driveName.isEmpty())
            continue;

        // 只对可以移除块设备进行处理
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blDev->drive()));
        if (drv && drv->removable())
            removableDrives.insert(driveName);
    }

    // 记录最后错误, 尽可能将能弹出的移动盘弹出
    bool allSuccess = true;
    foreach (const QString &drive, removableDrives) {
        if (!ejectDrive(drive))
            allSuccess = false;
    }

    return allSuccess;
}

QUrl DUMountManager::getMountPathForBlock(const QString &blkName)
{
    QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blkName));
    if (!blk)
        return QUrl();

    const QList<QByteArray> &mountPoints = blk->mountPoints();
    if (mountPoints.empty())
        return QUrl();

    return QUrl::fromLocalFile(mountPoints.first());
}

QList<QUrl> DUMountManager::getMountPathForDrive(const QString &driveName)
{
    QList<QUrl> urls;

    for (const QString &blkStr : DDiskManager::blockDevices({})) {
        QScopedPointer<DBlockDevice> blkd(DDiskManager::createBlockDevice(blkStr));
        if (blkd && blkd->drive() == driveName) {
            const QUrl &url = getMountPathForBlock(blkStr);
            if (url.isValid())
                urls << url;
        }
    }
    return urls;
}

QList<QUrl> DUMountManager::getMountPathForAllDrive()
{
    const QStringList &blockDevices = DDiskManager::blockDevices({});
    QList<QUrl> urls;
    foreach (const QString blkName, blockDevices) {
        const QUrl &url = getMountPathForBlock(blkName);
        urls << url;
    }
    return urls;
}

void DUMountManager::clearError()
{
    errorMsg = QString();
}

QString DUMountManager::getErrorMsg()
{
    return errorMsg;
}
