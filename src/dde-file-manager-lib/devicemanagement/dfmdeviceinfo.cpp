// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmdeviceinfo.h"
#include "dfmabstractdeviceinterface.h"
#include "dfminvaliddeviceinfo.h"
#include "dfmudisks2deviceinfo.h"
#include "dfmvfsdeviceinfo.h"

#include <QIcon>
#include <ddiskmanager.h>

#include <shutil/fileutils.h>

DFM_USE_NAMESPACE

DFMDeviceInfo::DFMDeviceInfo()
    : DFileInfo("", false)
{
    c_attachedDevice.reset(new DFMInvalidDeviceInfo());
}

DFMDeviceInfo::DFMDeviceInfo(const DUrl &url)
    : DFileInfo(url, false)
{
    using ClassType = DFMAbstractDeviceInterface::DeviceClassType;
    ClassType devType = ClassType::invalid;
    QString scheme = url.scheme();
    QString udisksDBusPath;
    QUrl gvfsUrl;

    if (scheme.isEmpty() || scheme == FILE_SCHEME) {
        // TODO: detect mounted list and check given url match a mountpoint?
        devType = ClassType::invalid;

        if (url.path().contains(QStringLiteral("/org/freedesktop/UDisks2/"))) {
            devType = ClassType::udisks2;
            udisksDBusPath = url.path();
        }
    }

    if (scheme == "udisks") {
        devType = ClassType::udisks2;
        udisksDBusPath = url.path();
    }

    if (scheme == DEVICE_SCHEME) {
        QString pathStr = url.path();
        if (!QUrl::fromUserInput(pathStr).isEmpty()) {
            devType = ClassType::gvfs;
            gvfsUrl = QUrl::fromUserInput(pathStr);
        } else {
            devType = ClassType::udisks2;
            udisksDBusPath = pathStr;
        }
    }

    if (url.isFTPFile() || url.isSFTPFile() || url.isSMBFile() || url.isMTPFile()) {
        devType = ClassType::gvfs;
        gvfsUrl = url;
    }

    switch (devType) {
    case ClassType::gvfs:
        setVfsDeviceInfo(DUrl(gvfsUrl));
        break;
    case ClassType::udisks2:
        setUdisks2DeviceInfo(udisksDBusPath);
        break;
    default:
        c_attachedDevice.reset(new DFMInvalidDeviceInfo());
        break;
    }
}

void DFMDeviceInfo::setUdisks2DeviceInfo(const QString &dbusPath)
{
    c_attachedDevice.reset(new DFMUdisks2DeviceInfo(dbusPath));
}

void DFMDeviceInfo::setVfsDeviceInfo(const DUrl &url)
{
    if (!url.scheme().isEmpty() && url.scheme() != FILE_SCHEME) {
        c_attachedDevice.reset(new DFMVfsDeviceInfo(url));
    }
}

bool DFMDeviceInfo::isValidDevice() const
{
    return c_attachedDevice->deviceClassType() != DFMAbstractDeviceInterface::invalid;
}

quint64 DFMDeviceInfo::freeBytes() const
{
    return attachedDeviceConst()->freeBytes();
}

quint64 DFMDeviceInfo::totalBytes() const
{
    return attachedDeviceConst()->totalBytes();
}

DFMDeviceInfo::MediaType DFMDeviceInfo::mediaType() const
{
    // TODO
    return unknown;
}

bool DFMDeviceInfo::exists() const
{
    return true;
}

QString DFMDeviceInfo::fileName() const
{
    return attachedDeviceConst()->name();
}

QString DFMDeviceInfo::fileDisplayName() const
{
    return attachedDeviceConst()->displayName();
}

bool DFMDeviceInfo::canRename() const
{
    return attachedDeviceConst()->canRename();
}

bool DFMDeviceInfo::isReadable() const
{
    return true;
}

bool DFMDeviceInfo::isWritable() const
{
    return attachedDeviceConst()->isReadOnly();
}

qint64 DFMDeviceInfo::size() const
{
    return static_cast<qint64>(attachedDeviceConst()->totalBytes());
}

int DFMDeviceInfo::filesCount() const
{
    QString mountPath = attachedDeviceConst()->mountpointPath();
    if (mountPath.isEmpty()) return 0;
    return FileUtils::filesCount(mountPath);
}

QIcon DFMDeviceInfo::fileIcon() const
{
    return QIcon::fromTheme(attachedDeviceConst()->iconName(), QIcon::fromTheme("drive-harddisk"));
}

bool DFMDeviceInfo::isDir() const
{
    return true;
}

DUrl DFMDeviceInfo::parentUrl() const
{
    return DUrl::fromComputerFile("/");
}

bool DFMDeviceInfo::canRedirectionFileUrl() const
{
    return attachedDeviceConst()->mountpointPath().isEmpty();
}

DUrl DFMDeviceInfo::redirectedFileUrl() const
{
    return DUrl::fromLocalFile(attachedDeviceConst()->mountpointPath());
}

DFMAbstractDeviceInterface *DFMDeviceInfo::attachedDevice()
{
    return c_attachedDevice.data();
}

const DFMAbstractDeviceInterface *DFMDeviceInfo::attachedDeviceConst() const
{
    return c_attachedDevice.data();
}
