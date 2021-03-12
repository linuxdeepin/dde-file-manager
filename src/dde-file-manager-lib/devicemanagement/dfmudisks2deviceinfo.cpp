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


#include "dfmudisks2deviceinfo.h"

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <QStorageInfo>

#include <shutil/fileutils.h>

DFM_BEGIN_NAMESPACE

DFMUdisks2DeviceInfo::DFMUdisks2DeviceInfo(const DBlockDevice *blockDevicePointer)
    : deviceDBusId(blockDevicePointer->path())
    , mountPoint(blockDevicePointer->mountPoints().first())
{
    c_blockDevice.reset(DDiskManager::createBlockDevice(deviceDBusId)); // not take the ownership of the passed pointer.
}

DFMUdisks2DeviceInfo::DFMUdisks2DeviceInfo(const QString &dbusPath)
{
    c_blockDevice.reset(DDiskManager::createBlockDevice(dbusPath)); // not take the ownership of the passed pointer.
}

void DFMUdisks2DeviceInfo::mount()
{
    blockDevice()->mount({});
}

bool DFMUdisks2DeviceInfo::unmountable()
{
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blockDevice()->drive()));
    return diskDev->removable();
}

void DFMUdisks2DeviceInfo::unmount()
{
    blockDevice()->unmount({});
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blockDevice()->drive()));
    if (diskDev->optical()) { // is optical
        if (diskDev->ejectable()) {
            diskDev->eject({});
        }
    }
}

bool DFMUdisks2DeviceInfo::ejectable()
{
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blockDevice()->drive()));
    return diskDev->ejectable();
}

void DFMUdisks2DeviceInfo::eject()
{
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blockDevice()->drive()));
    if (diskDev->ejectable()) {
        diskDev->eject({});
    }
}

bool DFMUdisks2DeviceInfo::isReadOnly() const
{
    return blockDeviceConst()->readOnly();
}

QString DFMUdisks2DeviceInfo::name() const
{
    if (blockDeviceConst()->isValid()) {
        return blockDeviceConst()->idLabel();
    }

    return QString();
}

bool DFMUdisks2DeviceInfo::canRename() const
{
    return blockDeviceConst()->canSetLabel();
}

QString DFMUdisks2DeviceInfo::displayName() const
{
    static QMap<QString, const char *> i18nMap {
        {"data", "Data Disk"}
    };

    QStorageInfo storage_info(mountPoint);
    bool hasLabelName = true;
    QString result;

    if (blockDeviceConst()->isValid()) {
        QString devName = blockDeviceConst()->idLabel();
        if (devName.isEmpty()) {
            hasLabelName = false;
            devName = qApp->translate("DeepinStorage", "%1 Volume").arg(FileUtils::formatSize(static_cast<qint64>(blockDeviceConst()->size())));
        }

        // Deepin i10n Label text (_dde_text):
        if (devName.startsWith(ddeI18nSym)) {
            QString i18nKey = devName.mid(ddeI18nSym.size(), devName.size() - ddeI18nSym.size());
            devName = qApp->translate("DeepinStorage", i18nMap.value(i18nKey, i18nKey.toUtf8().constData()));
        }

        result = devName;
    }

    if (storage_info.isValid()) {
        if (!hasLabelName) {
            qint64 bytesTotal = storage_info.bytesTotal();
            result = qApp->translate("DeepinStorage", "%1 Volume").arg(FileUtils::formatSize(bytesTotal));
        }
    }

    return result;
}

QString DFMUdisks2DeviceInfo::iconName() const
{
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blockDeviceConst()->drive()));

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

bool DFMUdisks2DeviceInfo::deviceUsageValid() const
{
    QStorageInfo storage_info(mountPoint);
    return storage_info.isValid();
}

quint64 DFMUdisks2DeviceInfo::availableBytes() const
{
    QStorageInfo storage_info(mountPoint);

    if (storage_info.isValid()) {
        return static_cast<quint64>(storage_info.bytesAvailable());
    }

    return static_cast<quint64>(-1);
}

quint64 DFMUdisks2DeviceInfo::freeBytes() const
{
    QStorageInfo storage_info(mountPoint);

    if (storage_info.isValid()) {
        return static_cast<quint64>(storage_info.bytesFree());
    }

    return static_cast<quint64>(-1);
}

quint64 DFMUdisks2DeviceInfo::totalBytes() const
{
    QStorageInfo storage_info(mountPoint);

    if (storage_info.isValid()) {
        return static_cast<quint64>(storage_info.bytesTotal());
    }

    return 0;
}

QString DFMUdisks2DeviceInfo::mountpointPath() const
{
    return mountPoint;
}

DFMAbstractDeviceInterface::DeviceClassType DFMUdisks2DeviceInfo::deviceClassType()
{
    return udisks2;
}

DBlockDevice *DFMUdisks2DeviceInfo::blockDevice()
{
    return c_blockDevice.data();
}

const DBlockDevice *DFMUdisks2DeviceInfo::blockDeviceConst() const
{
    return c_blockDevice.data();
}

DFM_END_NAMESPACE
