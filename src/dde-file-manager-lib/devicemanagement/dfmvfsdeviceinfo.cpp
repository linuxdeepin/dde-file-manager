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
