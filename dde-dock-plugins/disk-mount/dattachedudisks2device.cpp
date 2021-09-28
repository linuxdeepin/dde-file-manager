/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include "dattachedudisks2device.h"
#include "diskcontrolitem.h"
#include "diskcontrolwidget.h"

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <QStorageInfo>
#include <QtConcurrentRun>

DFM_USE_NAMESPACE

static std::atomic_bool processingRemove = ATOMIC_VAR_INIT(false);

/*!
 * \brief removeDevice  detach the device and its sibling devices and eject/poweroff the drive
 * \param blkPath       a string refers to the dbus path of this device
 */
static void removeRelatedBlocks(QString blkPath) {
    if (processingRemove)
        return;
    processingRemove.store(true);
    // get the disk path (a '/org/free...../sda' string, not end with a number)
    QString path = blkPath;
    path.remove(QRegExp("[0-9]*$"));

    QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blkPath));
    if (!blk) {
        processingRemove.store(false);
        return;
    }

    const QStringList &blks = DDiskManager::blockDevices({});
    for (const auto &blkItem: blks) {
        if (!blkItem.contains(path))
            continue;

        QScopedPointer<DBlockDevice> pblk(DDiskManager::createBlockDevice(blkItem));
        if (pblk->drive() != blk->drive())
            continue;

        if (pblk->mountPoints().count() > 0) {
            pblk->unmount({});
            QDBusError lastErr = pblk->lastError();
            if (lastErr.type() != QDBusError::NoError) {
                qCritical() << "device [" << pblk->idLabel() << "] unmount failed: " << lastErr.message();
                DiskControlWidget::NotifyMsg(DiskControlWidget::tr("%1 is busy and cannot be unmounted now").arg(pblk->idLabel().isEmpty() ? QString(pblk->device()) : pblk->idLabel()));
                processingRemove.store(false);
                return;
            }
        }
    }

    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
    if (drv->optical())
        drv->eject({});
    else
        drv->powerOff({});
    QDBusError err = drv->lastError();
    if (err.type() != QDBusError::NoError) {
        qCritical() << "device [" << drv->path() << "] poweroff failed: " << err.message();
        DiskControlWidget::NotifyMsg(DiskControlWidget::tr("Disk is busy, cannot remove now"));
    }
    processingRemove.store(false);
}


/*!
 * \class DAttachedUdisks2Device
 *
 * \brief An attached (mounted) block device (partition)
 */
DAttachedUdisks2Device::DAttachedUdisks2Device(const DBlockDevice *blockDevicePointer)
{
    QByteArrayList mountPoints = blockDevicePointer->mountPoints();
    mountPoint = mountPoints.isEmpty() ? "" : mountPoints.first();
    deviceDBusId = blockDevicePointer->path();
    c_blockDevice.reset(DDiskManager::createBlockDevice(deviceDBusId)); // not take the ownership of the passed pointer.
}

bool DAttachedUdisks2Device::isValid()
{
    return !c_blockDevice.isNull();
}

bool DAttachedUdisks2Device::detachable()
{
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blockDevice()->drive()));
    return diskDev->removable();
}

void DAttachedUdisks2Device::detach()
{
    QtConcurrent::run([this]() {
        removeRelatedBlocks(this->deviceDBusId);
    });
}

QString DAttachedUdisks2Device::displayName()
{
    static QMap<QString, const char *> i18nMap {
        {"data", "Data Disk"}
    };

    QStorageInfo storage_info(mountPoint);
    bool hasLabelName = true;
    QString result;

    if (blockDevice()->isValid()) {
        QString devName = blockDevice()->idLabel();
        if (devName.isEmpty()) {
            hasLabelName = false;
            devName = qApp->translate("DeepinStorage", "%1 Volume").arg(DiskControlItem::formatDiskSize(blockDevice()->size()));
        }

        // Deepin i10n Label text (_dde_text):
        if (devName.startsWith(ddeI18nSym)) {
            QString i18nKey = devName.mid(ddeI18nSym.size(), devName.size() - ddeI18nSym.size());
            devName = qApp->translate("DeepinStorage", i18nMap.value(i18nKey, i18nKey.toUtf8().constData()));
        }

        result = devName;
    } else if (storage_info.isValid()) {
        if (!hasLabelName) {
            qint64 bytesTotal = storage_info.bytesTotal();
            result = qApp->translate("DeepinStorage", "%1 Volume").arg(DiskControlItem::formatDiskSize(bytesTotal));
        }
    }

    return result;
}

bool DAttachedUdisks2Device::deviceUsageValid()
{
    QStorageInfo storage_info(mountPoint);
    return storage_info.isValid();
}

QPair<quint64, quint64> DAttachedUdisks2Device::deviceUsage()
{
    QStorageInfo storage_info(mountPoint);

    if (storage_info.isValid()) {
        qint64 bytesTotal = storage_info.bytesTotal();
        qint64 bytesFree = storage_info.bytesFree();
        return QPair<quint64, quint64>(bytesFree, bytesTotal);
    }

    return QPair<quint64, quint64>(-1, 0);
}

QString DAttachedUdisks2Device::iconName()
{
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blockDevice()->drive()));

    bool isDvd = diskDev->optical();
    bool isRemovable = diskDev->removable();
    QString iconName = QStringLiteral("drive-harddisk");

    if (isRemovable) {
        iconName = QStringLiteral("drive-removable-media-usb");
    }

    if (isDvd) {
        iconName = QStringLiteral("media-optical");
    }

    return iconName;
}

QUrl DAttachedUdisks2Device::mountpointUrl()
{
    return QUrl::fromLocalFile(mountPoint);
}

QUrl DAttachedUdisks2Device::accessPointUrl()
{
    QUrl url = mountpointUrl();
    auto blk = blockDevice();
    if (blk) {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (drv && drv->optical()) {
            url = DUrl::fromBurnFile(QString(blk->device()) + "/" + BURN_SEG_ONDISC + "/");
        }
    }
    return url;
}

DBlockDevice *DAttachedUdisks2Device::blockDevice()
{
    return c_blockDevice.data();
}
