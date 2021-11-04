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
#include "pluginsidecar.h"
#include "sizeformathelper.h"

#include <QCoreApplication>
#include <QVariantMap>

static const char *const BURN_SEG_ONDISC = "disc_files";
static const char *const BURN_SCHEME = "burn";

/*!
 * \brief makeBurnFileUrl as `DUrl::fromBurnFile` in old dde-file-manager
 * \param device like /dev/sr0
 * \return burn url (like 'burn:///dev/sr0/disc_files/')
 */
static QUrl makeBurnFileUrl(const QString &device)
{
    QUrl url;
    QString virtualPath(device + "/" + BURN_SEG_ONDISC + "/");
    url.setScheme(BURN_SCHEME);
    url.setPath(virtualPath);
    return url;
}

/*!
 * \class DAttachedBlockDevice
 *
 * \brief DAttachedBlockDevice implemented the
 * `DAttachedDeviceInterface` interface for block devices
 */

DAttachedBlockDevice::DAttachedBlockDevice(const QString &id)
    : DAttachedDevice(id)
{

}

DAttachedBlockDevice::~DAttachedBlockDevice()
{

}

bool DAttachedBlockDevice::isValid()
{
    if (data.value("id").toString().isEmpty())
        return false;
    if (!data.value("has_filesystem").toBool())
        return false;
    if (data.value("mountpoints").toStringList().isEmpty())
        return false;
    if (data.value("hint_ignore").toBool())
        return false;

    return true;
}

void DAttachedBlockDevice::detach()
{
    SidecarInstance.instance().invokeDetachBlockDevice(data.value("id").toString());
}

bool DAttachedBlockDevice::detachable()
{
    return data.value("removable").toBool();
}

QString DAttachedBlockDevice::displayName()
{
    QString result;

    static QMap<QString, const char *> i18nMap {
        {"data", "Data Disk"}
    };

    qint64 totalSize = data.value("size_total").toLongLong();
    if (isValid()) {
        QString devName = data.value("id_label").toString();
        if (devName.isEmpty()) {
            QString name = SizeFormatHelper::formatDiskSize(static_cast<quint64>(totalSize));
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
    return data.value("size_total").toLongLong() > 0;
}

QPair<quint64, quint64> DAttachedBlockDevice::deviceUsage()
{
    if (deviceUsageValid()) {
        bool optical = data.value("optical").toBool();
        qint64 bytesTotal = 0;
        qint64 bytesFree = 0;
        if (optical) {
            // TODO(zhangs): make a temp method read optical size for old dde-file-manager
        } else {
            bytesTotal = data.value("size_total").toLongLong();
            bytesFree = data.value("size_free").toLongLong();
        }
        return QPair<quint64, quint64>(static_cast<quint64>(bytesFree), static_cast<quint64>(bytesTotal));
    }
    return QPair<quint64, quint64>();
}

QString DAttachedBlockDevice::iconName()
{
    bool optical = data.value("optical").toBool();
    bool removable = data.value("removable").toBool();
    QString iconName = QStringLiteral("drive-harddisk");

    if (removable)
        iconName = QStringLiteral("drive-removable-media-usb");

    if (optical)
        iconName = QStringLiteral("media-optical");

    return QString();
}

QUrl DAttachedBlockDevice::mountpointUrl()
{
    return QUrl::fromLocalFile(data.value("mountpoint").toString());
}

QUrl DAttachedBlockDevice::accessPointUrl()
{
    QUrl url = mountpointUrl();
    bool optical = data.value("optical").toBool();

    if (optical) {
        QString &&device = data.value("device").toString();
        url = makeBurnFileUrl(device);
    }

    return url;
}

void DAttachedBlockDevice::query()
{
    data = SidecarInstance.invokeQueryBlockDeviceInfo(deviceId);
}
