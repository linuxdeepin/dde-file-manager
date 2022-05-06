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
#include "dattachedblockdevice.h"
#include "sizeformathelper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <QCoreApplication>
#include <QVariantMap>
#include <QDebug>

static const char *const kBurnSegOndisc = "disc_files";

using namespace GlobalServerDefines;

/*!
 * \brief makeBurnFileUrl as `DUrl::fromBurnFile` in old dde-file-manager
 * \param device like /dev/sr0
 * \return burn url (like 'burn:///dev/sr0/disc_files/')
 */
static QUrl makeBurnFileUrl(const QString &device)
{
    QUrl url;
    QString virtualPath(device + "/" + kBurnSegOndisc + "/");
    url.setScheme(DFMBASE_NAMESPACE::Global::kBurn);
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
    initializeConnect();
}

DAttachedBlockDevice::~DAttachedBlockDevice()
{
}

bool DAttachedBlockDevice::isValid()
{
    if (qvariant_cast<QString>(data.value(DeviceProperty::kId)).isEmpty())
        return false;
    if (!qvariant_cast<bool>(data.value(DeviceProperty::kHasFileSystem)))
        return false;
    if (qvariant_cast<QString>(data.value(DeviceProperty::kMountPoint)).isEmpty())
        return false;
    if (qvariant_cast<bool>(data.value(DeviceProperty::kHintIgnore)))
        return false;

    return true;
}

void DAttachedBlockDevice::detach()
{
    DevProxyMng->detachBlockDevice(qvariant_cast<QString>(data.value(DeviceProperty::kId)));
}

bool DAttachedBlockDevice::detachable()
{
    return data.value(DeviceProperty::kRemovable).toBool();
}

QString DAttachedBlockDevice::displayName()
{
    QString result;

    static QMap<QString, const char *> i18nMap {
        { "data", "Data Disk" }
    };

    qint64 totalSize { qvariant_cast<qint64>(data.value(DeviceProperty::kSizeTotal)) };
    if (isValid()) {
        QString devName { qvariant_cast<QString>(data.value(DeviceProperty::kIdLabel)) };
        if (devName.isEmpty()) {
            QString name { SizeFormatHelper::formatDiskSize(static_cast<quint64>(totalSize)) };
            devName = qApp->translate("DeepinStorage", "%1 Volume").arg(name);
        }

        // Deepin i10n Label text (_dde_text):
        if (devName.startsWith(ddeI18nSym)) {
            QString &&i18nKey = devName.mid(ddeI18nSym.size(), devName.size() - ddeI18nSym.size());
            devName = qApp->translate("DeepinStorage", i18nMap.value(i18nKey, i18nKey.toUtf8().constData()));
        }

        result = devName;
    } else if (totalSize > 0) {
        QString name = SizeFormatHelper::formatDiskSize(static_cast<quint64>(totalSize));
        result = qApp->translate("DeepinStorage", "%1 Volume").arg(name);
    }

    return result;
}

bool DAttachedBlockDevice::deviceUsageValid()
{
    return qvariant_cast<qint64>(data.value(DeviceProperty::kSizeTotal)) > 0;
}

QPair<quint64, quint64> DAttachedBlockDevice::deviceUsage()
{
    if (deviceUsageValid()) {
        qint64 bytesTotal { 0 };
        qint64 bytesFree { 0 };

        bytesTotal = qvariant_cast<qint64>(data.value(DeviceProperty::kSizeTotal));
        bytesFree = qvariant_cast<qint64>(data.value(DeviceProperty::kSizeFree));

        return QPair<quint64, quint64>(static_cast<quint64>(bytesFree), static_cast<quint64>(bytesTotal));
    }
    return QPair<quint64, quint64>(0, 0);
}

QString DAttachedBlockDevice::iconName()
{
    bool optical { qvariant_cast<bool>(data.value(DeviceProperty::kOptical)) };
    bool removable { qvariant_cast<bool>(data.value(DeviceProperty::kRemovable)) };
    bool decryptedDev = qvariant_cast<QString>(data.value(DeviceProperty::kCryptoBackingDevice)) != "/";
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
    return QUrl::fromLocalFile(data.value(DeviceProperty::kMountPoint).toString());
}

QUrl DAttachedBlockDevice::accessPointUrl()
{
    QUrl url { mountpointUrl() };
    bool optical { qvariant_cast<bool>(data.value(DeviceProperty::kOptical)) };

    if (optical) {
        QString device { qvariant_cast<QString>(data.value(DeviceProperty::kDevice)) };
        url = makeBurnFileUrl(device);
    }

    return url;
}

void DAttachedBlockDevice::query()
{
    data = DevProxyMng->queryBlockInfo(deviceId);
}

void DAttachedBlockDevice::initializeConnect()
{
    //    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::SizeUsedChanged, this, &DAttachedBlockDevice::onSizeChanged);
}

void DAttachedBlockDevice::onSizeChanged(const QString &id, qint64 total, qint64 free)
{
    QString thisId = qvariant_cast<QString>(data.value(DeviceProperty::kId));
    if (thisId == id) {
        qInfo() << "[disk-mout] Update device: " << thisId << "size, total: " << total << "free: " << free;
        data[DeviceProperty::kSizeTotal] = total;
        data[DeviceProperty::kSizeFree] = free;
    }
}
