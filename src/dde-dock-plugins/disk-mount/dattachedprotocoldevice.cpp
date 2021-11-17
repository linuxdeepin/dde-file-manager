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
#include "dattachedprotocoldevice.h"
#include "pluginsidecar.h"

#include <QVariantMap>

/*!
 * \class DAttachedProtocolDevice
 *
 * \brief DAttachedProtocolDevice implemented the
 * `DAttachedDeviceInterface` interface for protocol devices
 */

DAttachedProtocolDevice::DAttachedProtocolDevice(const QString &id, QObject *parent)
    : QObject(parent), DAttachedDevice(id)
{
}

DAttachedProtocolDevice::~DAttachedProtocolDevice()
{
}

bool DAttachedProtocolDevice::isValid()
{
    // TODO(zhans)
    return true;
}

void DAttachedProtocolDevice::detach()
{
    SidecarInstance.instance().invokeDetachProtocolDevice(data.value("id").toString());
}

bool DAttachedProtocolDevice::detachable()
{
    // TODO(zhans)
    return true;
}

QString DAttachedProtocolDevice::displayName()
{
    // TODO(zhans)
    return QString();
}

bool DAttachedProtocolDevice::deviceUsageValid()
{
    // TODO(zhans)
    return true;
}

QPair<quint64, quint64> DAttachedProtocolDevice::deviceUsage()
{
    // TODO(zhans)
    return QPair<quint64, quint64>();
}

QString DAttachedProtocolDevice::iconName()
{
    // TODO(zhans)
    return QString();
}

QUrl DAttachedProtocolDevice::mountpointUrl()
{
    // TODO(zhans)
    return QUrl();
}

QUrl DAttachedProtocolDevice::accessPointUrl()
{
    // TODO(zhans)
    return QUrl();
}

void DAttachedProtocolDevice::query()
{
    data = SidecarInstance.invokeQueryProtocolDeviceInfo(deviceId);
}
