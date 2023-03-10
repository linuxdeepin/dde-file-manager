// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dattachedprotocoldevice.h"
#include "devicewatcherlite.h"
#include "utils/dockutils.h"

#include <QVariantMap>
#include <QIcon>

#include <dfm-mount/dprotocoldevice.h>

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
    return device && !device->mountPoint().isEmpty();
}

void DAttachedProtocolDevice::detach()
{
    DeviceWatcherLite::instance()->detachProtocolDevice(deviceId);
}

bool DAttachedProtocolDevice::detachable()
{
    return true;
}

QString DAttachedProtocolDevice::displayName()
{
    if (!device)
        return tr("Unknown");

    QString devName = device->displayName();
    QString host, share;
    if (smb_utils::parseSmbInfo(devName, host, share))
        devName = tr("%1 on %2").arg(share).arg(host);
    return devName;
}

bool DAttachedProtocolDevice::deviceUsageValid()
{
    return true;
}

QPair<quint64, quint64> DAttachedProtocolDevice::deviceUsage()
{
    qint64 bytesTotal = device->sizeTotal();
    qint64 bytesFree = device->sizeFree();
    return QPair<quint64, quint64>(static_cast<quint64>(bytesFree), static_cast<quint64>(bytesTotal));
}

QString DAttachedProtocolDevice::iconName()
{
    if (!device)
        return "drive-network";

    auto iconLst = device->deviceIcons();
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
    if (!device)
        return QUrl("computer:///");
    return QUrl::fromLocalFile(device->mountPoint());
}

QUrl DAttachedProtocolDevice::accessPointUrl()
{
    return mountpointUrl();
}

void DAttachedProtocolDevice::query()
{
    device = DeviceWatcherLite::instance()->createProtocolDevicePtr(deviceId);
}
