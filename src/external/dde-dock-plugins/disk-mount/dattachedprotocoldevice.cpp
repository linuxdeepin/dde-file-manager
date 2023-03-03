// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dattachedprotocoldevice.h"

#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

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
    DevProxyMng->detachProtocolDevice(data.value(DeviceProperty::kId).toString());
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
    data = DevProxyMng->queryProtocolInfo(deviceId);
}
