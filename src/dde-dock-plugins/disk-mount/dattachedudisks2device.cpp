/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "dattachedudisks2device.h"
#include "diskcontrolitem.h"
#include "diskcontrolwidget.h"
#include "interfaces/dumountmanager.h"

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <QStorageInfo>
#include <QtConcurrentRun>

DFM_USE_NAMESPACE

ErrorHandleInfc::~ErrorHandleInfc()
{
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
    DBlockDevice *blkDevice = blockDevice();
    if (!blkDevice) {
        qWarning() << "blockDevice() got null!";
        return;
    }
    const QString &path = blkDevice->path();
    QtConcurrent::run([path]() {
        DUMountManager umountManager;
        const QString &drive = umountManager.getDriveName(path);
        if (!umountManager.ejectDrive(drive))
            DiskControlWidget::NotifyMsg(umountManager.getErrorMsg());
    });
}

QString DAttachedUdisks2Device::displayName()
{
    static QMap<QString, const char *> i18nMap {
        {"data", "Data Disk"}
    };

    QStorageInfo storage_info(mountPoint);
    QString result;

    if (blockDevice()->isValid()) {
        QString devName = blockDevice()->idLabel();
        if (devName.isEmpty()) {
            devName = qApp->translate("DeepinStorage", "%1 Volume").arg(DiskControlItem::formatDiskSize(blockDevice()->size()));
        }

        // Deepin i10n Label text (_dde_text):
        if (devName.startsWith(ddeI18nSym)) {
            QString i18nKey = devName.mid(ddeI18nSym.size(), devName.size() - ddeI18nSym.size());
            devName = qApp->translate("DeepinStorage", i18nMap.value(i18nKey, i18nKey.toUtf8().constData()));
        }

        result = devName;
    } else if (storage_info.isValid()) {
        qint64 bytesTotal = storage_info.bytesTotal();
        result = qApp->translate("DeepinStorage", "%1 Volume").arg(DiskControlItem::formatDiskSize(static_cast<quint64>(bytesTotal)));
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
        quint64 bytesTotal = blockDevice()->size();
        qint64 bytesFree = storage_info.bytesAvailable();
        return QPair<quint64, quint64>(static_cast<quint64>(bytesFree), static_cast<quint64>(bytesTotal));
    }

    return QPair<quint64, quint64>(0, 0);
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
