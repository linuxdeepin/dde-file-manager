// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blockdevhelper.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <fcntl.h>

FILE_ENCRYPT_USE_NS

quint64 blockdev_helper::devDeviceSize(const QString &phyDev)
{
    qInfo() << "[blockdev_helper::devDeviceSize] Getting device size for:" << phyDev;
    
    int fd = open(phyDev.toStdString().c_str(), O_RDONLY, 0);
    if (fd < 0) {
        qCritical() << "[blockdev_helper::devDeviceSize] Failed to open device for reading size:" << phyDev << "error:" << strerror(errno);
        return 0;
    }

    quint64 size = 0;
    int r = ioctl(fd, BLKGETSIZE64, &size);
    if (r < 0) {
        qCritical() << "[blockdev_helper::devDeviceSize] Failed to read device size via ioctl:" << phyDev << "error:" << strerror(errno);
    } else {
        qInfo() << "[blockdev_helper::devDeviceSize] Device size retrieved successfully:" << phyDev << "size:" << size << "bytes";
    }
    close(fd);
    return size;
}

quint64 blockdev_helper::devBlockSize(const QString &phyDev)
{
    const int BlockSectorSize = 512;
    return devDeviceSize(phyDev) / BlockSectorSize;
}

DevPtr blockdev_helper::createDevPtr(const QString &dev)
{
    qInfo() << "[blockdev_helper::createDevPtr] Creating device pointer for:" << dev;
    
    auto objpath = resolveDevObjPath(dev);
    if (objpath.isEmpty()) {
        qWarning() << "[blockdev_helper::createDevPtr] Failed to resolve object path for device:" << dev;
        return nullptr;
    }
    
    qInfo() << "[blockdev_helper::createDevPtr] Object path resolved:" << objpath << "for device:" << dev;
    return createDevPtr2(objpath);
}

int blockdev_helper::devCryptVersion(const QString &dev)
{
    qInfo() << "[blockdev_helper::devCryptVersion] Checking encryption version for device:" << dev;
    
    auto blkDev = createDevPtr(dev);
    if (!blkDev) {
        qCritical() << "[blockdev_helper::devCryptVersion] Failed to create block device handler for:" << dev;
        return kVersionUnknown;
    }

    const QString &idType = blkDev->getProperty(dfmmount::Property::kBlockIDType).toString();
    const QString &idVersion = blkDev->getProperty(dfmmount::Property::kBlockIDVersion).toString();

    qInfo() << "[blockdev_helper::devCryptVersion] Device properties - ID type:" << idType << "ID version:" << idVersion << "for device:" << dev;

    if (idType == "crypto_LUKS") {
        if (idVersion == "1") {
            qInfo() << "[blockdev_helper::devCryptVersion] Device is LUKS1 encrypted:" << dev;
            return kVersionLUKS1;
        }
        if (idVersion == "2") {
            qInfo() << "[blockdev_helper::devCryptVersion] Device is LUKS2 encrypted:" << dev;
            return kVersionLUKS2;
        }
        qWarning() << "[blockdev_helper::devCryptVersion] Unknown LUKS version for device:" << dev << "version:" << idVersion;
        return kVersionLUKSUnknown;
    }

    if (blkDev->isEncrypted()) {
        qInfo() << "[blockdev_helper::devCryptVersion] Device is encrypted with unknown method:" << dev;
        return kVersionUnknown;
    }

    // TODO: this should be completed, not only LUKS encrypt.

    qInfo() << "[blockdev_helper::devCryptVersion] Device is not encrypted:" << dev;
    return kNotEncrypted;
}

QString blockdev_helper::resolveDevObjPath(const QString &source)
{
    qInfo() << "[blockdev_helper::resolveDevObjPath] Resolving object path for source:" << source;
    
    if (source.isEmpty()) {
        qWarning() << "[blockdev_helper::resolveDevObjPath] Empty source provided";
        return "";
    }

    auto mng = dfmmount::DDeviceManager::instance();
    if (!mng) {
        qCritical() << "[blockdev_helper::resolveDevObjPath] Failed to get device manager instance";
        return "";
    }
    
    auto monitor = mng->getRegisteredMonitor(dfmmount::DeviceType::kBlockDevice).objectCast<dfmmount::DBlockMonitor>();
    if (!monitor) {
        qCritical() << "[blockdev_helper::resolveDevObjPath] Failed to get block device monitor";
        return "";
    }

    QVariantMap devspec, options {};
    if (source.startsWith("/dev/")) {
        devspec.insert("path", source);
        qInfo() << "[blockdev_helper::resolveDevObjPath] Using device path specification:" << source;
    } else if (source.startsWith("UUID")) {
        auto src = source.mid(5);   // UUID=XXXXXXXXXX
        devspec.insert("uuid", src);
        qInfo() << "[blockdev_helper::resolveDevObjPath] Using UUID specification:" << src;
    } else if (source.startsWith("PARTUUID")) {
        auto src = source.mid(9);   // PARTUUID=XXXXXXXXXX
        devspec.insert("partuuid", src);
        qInfo() << "[blockdev_helper::resolveDevObjPath] Using PARTUUID specification:" << src;
    } else {
        qWarning() << "[blockdev_helper::resolveDevObjPath] Unsupported source format:" << source;
        return "";
    }

    auto ret = monitor->resolveDevice(devspec, options);
    return ret.isEmpty() ? "" : ret.first();
}

DevPtr blockdev_helper::createDevPtr2(const QString &objPath)
{
    qInfo() << "[blockdev_helper::createDevPtr2] Creating device pointer from object path:" << objPath;
    
    if (objPath.isEmpty()) {
        qWarning() << "[blockdev_helper::createDevPtr2] Empty object path provided";
        return nullptr;
    }
    
    auto mng = dfmmount::DDeviceManager::instance();
    if (!mng) {
        qCritical() << "[blockdev_helper::createDevPtr2] Failed to get device manager instance";
        return nullptr;
    }
    
    auto monitor = mng->getRegisteredMonitor(dfmmount::DeviceType::kBlockDevice)
                           .objectCast<dfmmount::DBlockMonitor>();
    if (!monitor) {
        qCritical() << "[blockdev_helper::createDevPtr2] Failed to get block device monitor";
        return nullptr;
    }

    auto devPtr = monitor->createDeviceById(objPath).objectCast<dfmmount::DBlockDevice>();
    if (devPtr) {
        qInfo() << "[blockdev_helper::createDevPtr2] Successfully created device pointer for object path:" << objPath;
    } else {
        qWarning() << "[blockdev_helper::createDevPtr2] Failed to create device pointer for object path:" << objPath;
    }
    
    return devPtr;
}

QString blockdev_helper::getUSecName(const QString &dmDev)
{
    qInfo() << "[blockdev_helper::getUSecName] Getting USec name for DM device:" << dmDev;
    
    auto ptr = blockdev_helper::createDevPtr(dmDev);
    if (!ptr) {
        qCritical() << "[blockdev_helper::getUSecName] Failed to create device object for:" << dmDev;
        return "";
    }

    auto symlinks = ptr->getProperty(dfmmount::Property::kBlockSymlinks).toStringList();
    qDebug() << "[blockdev_helper::getUSecName] Device symlinks:" << symlinks << "for device:" << dmDev;
    
    for (auto link : symlinks) {
        if (link.contains("usec-overlay")) {
            auto name = link;
            name = name.remove("/dev/disk/by-id/dm-name-");
            qInfo() << "[blockdev_helper::getUSecName] Found USec overlay name:" << name << "for device:" << dmDev;
            return name;
        }
    }
    
    qWarning() << "[blockdev_helper::getUSecName] No USec overlay name found for device:" << dmDev;
    return "";
}
