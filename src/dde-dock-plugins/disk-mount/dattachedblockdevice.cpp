/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "dattachedblockdevice.h"
#include "pluginsidecar.h"

#include <QVariantMap>

/*!
 * \class DAttachedBlockDevice
 *
 * \brief DAttachedBlockDevice implemented the
 * `DAttachedDeviceInterface` interface for block devices
 */

DAttachedBlockDevice::DAttachedBlockDevice(const QString &id)
    : DAttachedDevice(id)
{

}

DAttachedBlockDevice::~DAttachedBlockDevice()
{

}

bool DAttachedBlockDevice::isValid()
{
    // TODO(zhans): judge hasFileSystem mountPoints hintIgnore hintSystem
    return false;
}

bool DAttachedBlockDevice::detachable()
{
    // TODO(zhans)
    return true;
}

QString DAttachedBlockDevice::displayName()
{
    // TODO(zhans)
    return QString();
}

bool DAttachedBlockDevice::deviceUsageValid()
{
    // TODO(zhans)
    return true;
}

QPair<quint64, quint64> DAttachedBlockDevice::deviceUsage()
{
    if (iconName().simplified().toLower() == "media-optical") {
        // TODO(zhangs): optical device cannot read capacity
        // servier must read common data
        return QPair<quint64, quint64>();
    }

    // TODO(zhangs): query
    return QPair<quint64, quint64>();
}

QString DAttachedBlockDevice::iconName()
{
    // TODO(zhans)
    return QString();
}

QUrl DAttachedBlockDevice::mountpointUrl()
{
    // TODO(zhans)
    return QUrl();
}

QUrl DAttachedBlockDevice::accessPointUrl()
{
    // TODO(zhans)
    return QUrl();
}

void DAttachedBlockDevice::parse()
{
    const QVariantMap &map = SidecarInstance.invokeQueryBlockDeviceInfo(deviceId);
    // TODO(zhans) make data
}
