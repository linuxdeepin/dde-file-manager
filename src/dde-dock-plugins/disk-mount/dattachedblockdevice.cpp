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

/*!
 * \class DAttachedBlockDevice
 *
 * \brief DAttachedBlockDevice implemented the
 * `DAttachedDeviceInterface` interface for block devices
 */


DAttachedBlockDevice::DAttachedBlockDevice()
{

}

DAttachedBlockDevice::~DAttachedBlockDevice()
{

}

bool DAttachedBlockDevice::isValid()
{
    // TODO(zhans)
    return true;
}

bool DAttachedBlockDevice::detachable()
{
    // TODO(zhans)
    return true;
}

void DAttachedBlockDevice::detach()
{
    // TODO(zhans)
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
    // TODO(zhans)
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
