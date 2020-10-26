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

#include <dgiofile.h>
#include <dgiofileinfo.h>

/*!
 * \class DAttachedVfsDevice
 *
 * \brief An attached (mounted) virtual filesystem device from gio
 */


DAttachedVfsDevice::DAttachedVfsDevice(const QString mountpointPath)
{
    dgioMount.reset(DGioMount::createFromPath(mountpointPath));
}

bool DAttachedVfsDevice::isValid()
{
    return !dgioMount.isNull();
}

bool DAttachedVfsDevice::detachable()
{
    return dgioMount->canUnmount();
}

void DAttachedVfsDevice::detach()
{
    dgioMount->unmount();
}

QString DAttachedVfsDevice::displayName()
{
    return dgioMount ? dgioMount->name() : QStringLiteral("-");
}

bool DAttachedVfsDevice::deviceUsageValid()
{
    if (dgioMount.isNull()) return false;

    QExplicitlySharedDataPointer<DGioFile> file = dgioMount->getRootFile();
    QExplicitlySharedDataPointer<DGioFileInfo> fsInfo = file->createFileSystemInfo("filesystem::*");

    return fsInfo;
}

QPair<quint64, quint64> DAttachedVfsDevice::deviceUsage()
{
    QExplicitlySharedDataPointer<DGioFile> file = dgioMount->getRootFile();
    QExplicitlySharedDataPointer<DGioFileInfo> fsInfo = file->createFileSystemInfo("filesystem::*");

    if (fsInfo) {
        return QPair<quint64, quint64>(fsInfo->fsFreeBytes(), fsInfo->fsTotalBytes());
    }

    return QPair<quint64, quint64>(0, 0);
}

QString DAttachedVfsDevice::iconName()
{
    QStringList iconList = dgioMount ? dgioMount->themedIconNames() : QStringList();

    if (iconList.empty()) {
        return QStringLiteral("drive-network");
    }

    return iconList.first();
}

QUrl DAttachedVfsDevice::mountpointUrl()
{
    QExplicitlySharedDataPointer<DGioFile> file = dgioMount->getRootFile();
    return QUrl::fromLocalFile(file->path());
}

QUrl DAttachedVfsDevice::accessPointUrl()
{
    return mountpointUrl();
}
