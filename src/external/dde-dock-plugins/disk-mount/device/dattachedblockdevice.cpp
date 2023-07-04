// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dattachedblockdevice.h"
#include "devicewatcherlite.h"
#include "utils/dockutils.h"

#include <QCoreApplication>
#include <QVariantMap>
#include <QDebug>
#include <QFile>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTime>

static const char *const kBurnSegOndisc = "disc_files";

/*!
 * \brief makeBurnFileUrl as `DUrl::fromBurnFile` in old dde-file-manager
 * \param device like /dev/sr0
 * \return burn url (like 'burn:///dev/sr0/disc_files/')
 */
static QUrl makeBurnFileUrl(const QString &device)
{
    QUrl url;
    QString virtualPath(device + "/" + kBurnSegOndisc + "/");
    url.setScheme("burn");
    url.setPath(virtualPath);
    return url;
}

/*!
 * \class DAttachedBlockDevice
 *
 * \brief DAttachedBlockDevice implemented the
 * `DAttachedDeviceInterface` interface for block devices
 */

DAttachedBlockDevice::DAttachedBlockDevice(const QString &id, QObject *parent)
    : QObject(parent), DAttachedDevice(id)
{
}

DAttachedBlockDevice::~DAttachedBlockDevice()
{
}

bool DAttachedBlockDevice::isValid()
{
    if (info.isEmpty())
        return false;

    if (info.value("IsEncrypted").toBool())
        return true;

    if (!info.value("HasFileSystem").toBool())
        return false;
    if (info.value("MountPoints").toStringList().isEmpty())
        return false;
    if (info.value("HintSystem").toBool() && info.value("CryptoBackingDevice").toString() == "/")
        return false;
    if (info.value("HintIgnore").toBool())
        return false;

    return true;
}

void DAttachedBlockDevice::detach()
{
    DeviceWatcherLite::instance()->detachBlockDevice(deviceId);
}

bool DAttachedBlockDevice::detachable()
{
    return info.value("Removable").toBool();
}

QString DAttachedBlockDevice::displayName()
{
    if (info.value("IsEncrypted").toBool()) {
        DAttachedBlockDevice clearDev(info.value("CleartextDevice").toString());
        clearDev.query();
        return clearDev.displayName();
    }

    QString result;

    static QMap<QString, const char *> i18nMap {
        { "data", "Data Disk" }
    };

    quint64 totalSize { info.value("SizeTotal").toULongLong() };
    if (isValid()) {
        QString devName { info.value("IdLabel").toString() };
        if (devName.isEmpty()) {
            QString name { size_format::formatDiskSize(totalSize) };
            devName = qApp->translate("DeepinStorage", "%1 Volume").arg(name);
        }

        // Deepin i10n Label text (_dde_text):
        if (devName.startsWith(ddeI18nSym)) {
            QString &&i18nKey = devName.mid(ddeI18nSym.size(), devName.size() - ddeI18nSym.size());
            devName = qApp->translate("DeepinStorage", i18nMap.value(i18nKey, i18nKey.toUtf8().constData()));
        }

        result = devName;
    } else if (totalSize > 0) {
        QString name = size_format::formatDiskSize(static_cast<quint64>(totalSize));
        result = qApp->translate("DeepinStorage", "%1 Volume").arg(name);
    }

    return result;
}

bool DAttachedBlockDevice::deviceUsageValid()
{
    return info.value("SizeTotal").toULongLong() > 0;
}

QPair<quint64, quint64> DAttachedBlockDevice::deviceUsage()
{
    if (info.value("IsEncrypted").toBool()) {
        DAttachedBlockDevice clearDev(info.value("CleartextDevice").toString());
        clearDev.query();
        return clearDev.deviceUsage();
    }

    quint64 free = (info.contains("SizeFree"))
            ? info.value("SizeFree").toULongLong()
            : info.value("SizeTotal").toULongLong();

    return { free,
             info.value("SizeTotal").toULongLong() };
}

QString DAttachedBlockDevice::iconName()
{
    bool optical { info.value("OpticalDrive").toBool() };
    bool removable { info.value("Removable").toBool() };
    bool decryptedDev { info.value("IsEncrypted").toBool() };
    QString iconName { QStringLiteral("drive-harddisk") };

    if (removable)
        iconName = QStringLiteral("drive-removable-media-usb");
    if (optical)
        iconName = QStringLiteral("media-optical");
    if (decryptedDev)
        iconName = QStringLiteral("drive-removable-media-encrypted");

    return iconName;
}

QUrl DAttachedBlockDevice::mountpointUrl()
{
    if (info.value("IsEncrypted").toBool()) {
        DAttachedBlockDevice clearDev(info.value("CleartextDevice").toString());
        clearDev.query();
        return clearDev.mountpointUrl();
    }

    QString mpt = info.value("MountPoint").toString();
    if (mpt.isEmpty())
        return QUrl("computer:///");
    return QUrl::fromLocalFile(mpt);
}

QUrl DAttachedBlockDevice::accessPointUrl()
{
    QUrl url { mountpointUrl() };
    bool optical { info.value("OpticalDrive").toBool() };

    if (optical) {
        QString devDesc { info.value("Device").toString() };
        url = makeBurnFileUrl(devDesc);
    }

    return url;
}

void DAttachedBlockDevice::query()
{
    QTime t;
    t.start();
    QDBusInterface iface("org.deepin.filemanager.server",
                         "/org/deepin/filemanager/server/DeviceManager",
                         "org.deepin.filemanager.server.DeviceManager", QDBusConnection::sessionBus());
    iface.setTimeout(3);
    QDBusReply<QVariantMap> ret = iface.callWithArgumentList(QDBus::CallMode::AutoDetect, "QueryBlockDeviceInfo", QList<QVariant> { deviceId, false });
    qInfo() << "query info of costs" << deviceId << t.elapsed();
    info = ret.value();
}
