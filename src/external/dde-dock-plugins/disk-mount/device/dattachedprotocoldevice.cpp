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
#include <QDBusInterface>
#include <QDBusReply>

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
    return !info.value("MountPoint").toString().isEmpty();
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
    QString devName = info.value("DisplayName").toString();
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
    return { info.value("SizeFree").toULongLong(),
             info.value("SizeTotal").toULongLong() };
}

QString DAttachedProtocolDevice::iconName()
{
    auto iconLst = info.value("DeviceIcon").toStringList();
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
    QString mpt = info.value("MountPoint").toString();
    if (mpt.isEmpty())
        return QUrl("computer:///");
    return QUrl::fromLocalFile(mpt);
}

QUrl DAttachedProtocolDevice::accessPointUrl()
{
    return mountpointUrl();
}

void DAttachedProtocolDevice::query()
{
    QTime t;
    t.start();
    QDBusInterface iface("org.deepin.filemanager.server",
                         "/org/deepin/filemanager/server/DeviceManager",
                         "org.deepin.filemanager.server.DeviceManager", QDBusConnection::sessionBus());
    iface.setTimeout(3);
    QDBusReply<QVariantMap> ret = iface.callWithArgumentList(QDBus::CallMode::AutoDetect, "QueryProtocolDeviceInfo", QList<QVariant> { deviceId, false });
    qInfo() << "query info of costs" << deviceId << t.elapsed();
    info = ret.value();
    qInfo() << "the queried info of protocol device:" << info;
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
                port = "445";
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
            port = "445";
        return true;
    }

    return false;
}
