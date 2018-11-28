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

#include "dattachedvfsdevice.h"

#include "dfmvfsdevice.h"

DFM_USE_NAMESPACE

/*!
 * \class DAttachedVfsDevice
 *
 * \brief An attached (mounted) virtual filesystem device from gio
 */


DAttachedVfsDevice::DAttachedVfsDevice(const QUrl mountpointUrl)
{
    vfsDevice.reset(DFMVfsDevice::create(mountpointUrl));
}

bool DAttachedVfsDevice::isValid()
{
    return !vfsDevice.isNull();
}

bool DAttachedVfsDevice::detachable()
{
    return vfsDevice->canDetach();
}

void DAttachedVfsDevice::detach()
{
    vfsDevice->detachAsync();
}

QString DAttachedVfsDevice::displayName()
{
    return vfsDevice ? vfsDevice->name() : QStringLiteral("-");
}

bool DAttachedVfsDevice::deviceUsageValid()
{
    return !vfsDevice.isNull();
}

QPair<quint64, quint64> DAttachedVfsDevice::deviceUsage()
{
    return QPair<quint64, quint64>(vfsDevice->freeBytes(), vfsDevice->totalBytes());
}

QString DAttachedVfsDevice::iconName()
{
    QStringList iconList = vfsDevice ? vfsDevice->iconList() : QStringList();

    if (iconList.empty()) {
        return QStringLiteral("drive-network");
    }

    return iconList.first();
}

QUrl DAttachedVfsDevice::mountpointUrl()
{
    return QUrl::fromLocalFile(vfsDevice->rootPath());
}
