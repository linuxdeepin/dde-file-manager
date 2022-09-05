// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmvfsdeviceinfo.h"

#include "dfmvfsdevice.h"

DFM_BEGIN_NAMESPACE

DFMVfsDeviceInfo::DFMVfsDeviceInfo(const QUrl mountpointUrl)
{
    c_vfsDevice.reset(DFMVfsDevice::create(mountpointUrl));
}

void DFMVfsDeviceInfo::mount()
{
    vfsDevice()->attach();
}

bool DFMVfsDeviceInfo::unmountable()
{
    return vfsDevice()->canDetach();
}

void DFMVfsDeviceInfo::unmount()
{
    vfsDevice()->detachAsync();
}

bool DFMVfsDeviceInfo::ejectable()
{
    return unmountable();
}

void DFMVfsDeviceInfo::eject()
{
    unmount();
}

QString DFMVfsDeviceInfo::name() const
{
    return vfsDeviceConst()->name();
}

bool DFMVfsDeviceInfo::canRename() const
{
    return false;
}

QString DFMVfsDeviceInfo::displayName() const
{
    return name();
}

QString DFMVfsDeviceInfo::iconName() const
{
    QList<QString> iconList = vfsDeviceConst()->iconList();

    if (iconList.empty()) {
        return QStringLiteral("drive-network");
    }

    return iconList.first();
}

bool DFMVfsDeviceInfo::deviceUsageValid() const
{
    return true;
}

quint64 DFMVfsDeviceInfo::availableBytes() const
{
    return vfsDeviceConst()->freeBytes();
}

quint64 DFMVfsDeviceInfo::freeBytes() const
{
    return vfsDeviceConst()->freeBytes();
}

quint64 DFMVfsDeviceInfo::totalBytes() const
{
    return vfsDeviceConst()->totalBytes();
}

QString DFMVfsDeviceInfo::mountpointPath() const
{
    return vfsDeviceConst()->rootPath();
}

DFMAbstractDeviceInterface::DeviceClassType DFMVfsDeviceInfo::deviceClassType()
{
    return gvfs;
}

bool DFMVfsDeviceInfo::isReadOnly() const
{
    return vfsDeviceConst()->isReadOnly();
}

DFMVfsDevice *DFMVfsDeviceInfo::vfsDevice()
{
    return c_vfsDevice.data();
}

const DFMVfsDevice *DFMVfsDeviceInfo::vfsDeviceConst() const
{
    return c_vfsDevice.data();
}

DFM_END_NAMESPACE
