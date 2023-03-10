// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dattachedblockdevice.h"
#include "devicewatcherlite.h"
#include "utils/dockutils.h"

#include <QCoreApplication>
#include <QVariantMap>
#include <QDebug>

#include <dfm-mount/dblockdevice.h>

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
    if (!device)
        return false;

    if (!device->hasFileSystem())
        return false;
    if (device->mountPoint().isEmpty())
        return false;
    if (device->hintSystem() || device->hintIgnore())
        return false;

    return true;
}

void DAttachedBlockDevice::detach()
{
    if (!device)
        return;

    DeviceWatcherLite::instance()->detachBlockDevice(deviceId);
}

bool DAttachedBlockDevice::detachable()
{
    return device && device->removable();
}

QString DAttachedBlockDevice::displayName()
{
    if (!device)
        return tr("Unknown");

    QString result;

    static QMap<QString, const char *> i18nMap {
        { "data", "Data Disk" }
    };

    qint64 totalSize { device->sizeTotal() };
    if (isValid()) {
        QString devName { device->idLabel() };
        if (devName.isEmpty()) {
            QString name { size_format::formatDiskSize(static_cast<quint64>(totalSize)) };
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
    return device && device->sizeTotal() > 0;
}

QPair<quint64, quint64> DAttachedBlockDevice::deviceUsage()
{
    if (deviceUsageValid()) {
        qint64 bytesTotal { device->sizeTotal() };
        qint64 bytesFree { device->sizeFree() };
        return QPair<quint64, quint64>(static_cast<quint64>(bytesFree), static_cast<quint64>(bytesTotal));
    }
    return QPair<quint64, quint64>(0, 0);
}

QString DAttachedBlockDevice::iconName()
{
    if (!device)
        return "drive-harddisk";

    bool optical { device->optical() };
    bool removable { device->removable() };
    bool decryptedDev = device->getProperty(DFMMOUNT::Property::kEncryptedCleartextDevice).toString() != "/";
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
    if (!device)
        return QUrl("computer:///");
    return QUrl::fromLocalFile(device->mountPoint());
}

QUrl DAttachedBlockDevice::accessPointUrl()
{
    if (!device)
        return QUrl("computer:///");

    QUrl url { mountpointUrl() };
    bool optical { device->optical() };

    if (optical) {
        QString devDesc { device->device() };
        url = makeBurnFileUrl(devDesc);
    }

    return url;
}

void DAttachedBlockDevice::query()
{
    device = DeviceWatcherLite::instance()->createBlockDevicePtr(deviceId);
}
