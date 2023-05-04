// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dattachedprotocoldevice.h"
#include "devicewatcherlite.h"
#include "utils/dockutils.h"

#include <QVariantMap>
#include <QIcon>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

#include <dfm-mount/dprotocoldevice.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

/*!
 * \class DAttachedProtocolDevice
 *
 * \brief DAttachedProtocolDevice implemented the
 * `DAttachedDeviceInterface` interface for protocol devices
 */

DAttachedProtocolDevice::DAttachedProtocolDevice(const QString &id, QObject *parent)
    : QObject(parent), DAttachedDevice(id)
{
    isNetworkDev = id.contains(QRegularExpression(R"(^(ftp|sftp|smb))"))
            || id.contains(QRegularExpression(R"(^file:///media/.*/smbmounts/smb-share)"));
    qDebug() << id << "isNetwork: " << isNetworkDev;
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
    if (!device)
        return { 0, 0 };

    if (isNetworkDev && QDateTime::currentSecsSinceEpoch() - lastNetCheck < 180)
        return { latestFreeSize, latestTotalSize };
    lastNetCheck = QDateTime::currentSecsSinceEpoch();

    // if network then check net
    if (isNetworkDev && !checkNetwork()) {
        qWarning() << "network is disconnecting, use latest usage info of" << deviceId;
        return { latestFreeSize, latestTotalSize };
    }

    latestTotalSize = static_cast<quint64>(device->sizeTotal());
    latestFreeSize = static_cast<quint64>(device->sizeFree());
    return { latestFreeSize, latestTotalSize };
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

bool DAttachedProtocolDevice::checkNetwork()
{
    QString host, port;
    if (!parseHostAndPort(host, port) || host.isEmpty())
        return false;

    qInfo() << "checking network connection..." << QString("%1:%2").arg(host).arg(port);
    addrinfo *result;
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;   // either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    char addressString[INET6_ADDRSTRLEN];
    const char *retval = nullptr;
    if (0 != getaddrinfo(host.toUtf8().toStdString().c_str(), port.toUtf8().toStdString().c_str(), &hints, &result)) {
        return false;
    }
    for (addrinfo *addr = result; addr != nullptr; addr = addr->ai_next) {
        int handle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (handle == -1) {
            continue;
        }
        if (::connect(handle, addr->ai_addr, addr->ai_addrlen) != -1) {
            switch (addr->ai_family) {
            case AF_INET:
                retval = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in *>(addr->ai_addr)->sin_addr), addressString, INET_ADDRSTRLEN);
                break;
            case AF_INET6:
                retval = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in6 *>(addr->ai_addr)->sin6_addr), addressString, INET6_ADDRSTRLEN);
                break;
            default:
                // unknown family
                retval = nullptr;
            }
            close(handle);
            break;
        }
    }
    freeaddrinfo(result);
    return retval != nullptr;
}

bool DAttachedProtocolDevice::parseHostAndPort(QString &host, QString &port)
{
    if (deviceId.contains(QRegularExpression(R"(^(ftp|sftp|smb))"))) {
        QUrl url(deviceId);
        host = url.host();
        int nPort = url.port();
        if (nPort < 0) {
            if (deviceId.startsWith("ftp")) {
                port = "21";
            } else if (deviceId.startsWith("sftp")) {
                port = "22";
            } else {
                port = "139";
            }
        } else {
            port = QString::number(nPort);
        }
        return true;
    }

    if (deviceId.contains(QRegularExpression(R"(^file:///media/.*/smbmounts/smb-share)"))) {
        QRegularExpression regx(R"(([:,]port=(?<port>\d*))?[,:]server=(?<host>[^/:,]+)(,share=(?<share>[^/:,]+))?)");
        auto match = regx.match(deviceId);
        if (!match.hasMatch())
            return false;

        host = match.captured("host");
        port = match.captured("port");
        if (port.isEmpty())
            port = "139";
        return true;
    }

    return false;
}
