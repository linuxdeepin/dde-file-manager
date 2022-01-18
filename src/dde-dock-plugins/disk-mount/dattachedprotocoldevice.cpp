/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "devicemanager.h"

#include <global_server_defines.h>
#include <QVariantMap>
#include <QIcon>

using namespace GlobalServerDefines;

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
    return !data.value(DeviceProperty::kMountPoint).toString().isEmpty();
}

void DAttachedProtocolDevice::detach()
{
    DeviceManagerInstance.instance().invokeDetachProtocolDevice(data.value(DeviceProperty::kId).toString());
}

bool DAttachedProtocolDevice::detachable()
{
    return true;
}

QString DAttachedProtocolDevice::displayName()
{
    return data.value(DeviceProperty::kDisplayName, tr("Unknown device")).toString();
}

bool DAttachedProtocolDevice::deviceUsageValid()
{
    return true;
}

QPair<quint64, quint64> DAttachedProtocolDevice::deviceUsage()
{
    qint64 bytesTotal = qvariant_cast<qint64>(data.value(DeviceProperty::kSizeTotal));
    qint64 bytesFree = qvariant_cast<qint64>(data.value(DeviceProperty::kSizeFree));
    return QPair<quint64, quint64>(static_cast<quint64>(bytesFree), static_cast<quint64>(bytesTotal));
}

QString DAttachedProtocolDevice::iconName()
{
    auto iconLst = data.value(DeviceProperty::kDeviceIcon).toStringList();
    for (auto name : iconLst) {
        auto icon = QIcon::fromTheme(name);
        if (!icon.isNull())
            return name;
    }
    auto iconName = QStringLiteral("drive-network");
    return iconName;
}

QUrl DAttachedProtocolDevice::mountpointUrl()
{
    return QUrl::fromLocalFile(data.value(DeviceProperty::kMountPoint).toString());
}

QUrl DAttachedProtocolDevice::accessPointUrl()
{
    return mountpointUrl();
}

void DAttachedProtocolDevice::query()
{
    data = DeviceManagerInstance.invokeQueryProtocolDeviceInfo(deviceId);
}
