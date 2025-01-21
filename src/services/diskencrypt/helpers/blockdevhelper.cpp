// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    int fd = open(phyDev.toStdString().c_str(), O_RDONLY, 0);
    if (fd < 0) {
        qWarning() << "cannot open device for read size!";
        return 0;
    }

    quint64 size = 0;
    int r = ioctl(fd, BLKGETSIZE64, &size);
    if (r < 0)
        qWarning() << "cannot read device size by ioctl!";
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
    auto objpath = resolveDevObjPath(dev);
    if (objpath.isEmpty()) return nullptr;
    return createDevPtr2(objpath);
}

int blockdev_helper::devCryptVersion(const QString &dev)
{
    auto blkDev = createDevPtr(dev);
    if (!blkDev) {
        qWarning() << "cannot create block device handler:" << dev;
        return kVersionUnknown;
    }

    const QString &idType = blkDev->getProperty(dfmmount::Property::kBlockIDType).toString();
    const QString &idVersion = blkDev->getProperty(dfmmount::Property::kBlockIDVersion).toString();

    if (idType == "crypto_LUKS") {
        if (idVersion == "1")
            return kVersionLUKS1;
        if (idVersion == "2")
            return kVersionLUKS2;
        return kVersionLUKSUnknown;
    }

    if (blkDev->isEncrypted())
        return kVersionUnknown;

    // TODO: this should be completed, not only LUKS encrypt.

    return kNotEncrypted;
}

QString blockdev_helper::resolveDevObjPath(const QString &source)
{
    if (source.isEmpty()) return "";

    auto mng = dfmmount::DDeviceManager::instance();
    if (!mng) return "";
    auto monitor = mng->getRegisteredMonitor(dfmmount::DeviceType::kBlockDevice).objectCast<dfmmount::DBlockMonitor>();
    if (!monitor) return "";

    QVariantMap devspec, options {};
    if (source.startsWith("/dev/")) {
        devspec.insert("path", source);
    } else if (source.startsWith("UUID")) {
        auto src = source.mid(5);   // UUID=XXXXXXXXXX
        devspec.insert("uuid", src);
    } else if (source.startsWith("PARTUUID")) {
        auto src = source.mid(9);   // PARTUUID=XXXXXXXXXX
        devspec.insert("partuuid", src);
    } else {
        return "";
    }

    auto ret = monitor->resolveDevice(devspec, options);
    return ret.isEmpty() ? "" : ret.first();
}

DevPtr blockdev_helper::createDevPtr2(const QString &objPath)
{
    if (objPath.isEmpty()) return nullptr;
    auto mng = dfmmount::DDeviceManager::instance();
    if (!mng) return nullptr;
    auto monitor = mng->getRegisteredMonitor(dfmmount::DeviceType::kBlockDevice)
                           .objectCast<dfmmount::DBlockMonitor>();
    if (!monitor) return nullptr;

    return monitor->createDeviceById(objPath)
            .objectCast<dfmmount::DBlockDevice>();
}
