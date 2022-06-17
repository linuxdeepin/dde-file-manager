/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "deviceutils.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/utils/finallyutil.h"

#include <QVector>
#include <QDebug>
#include <QRegularExpressionMatch>

#include <libmount.h>

DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines::DeviceProperty;

QString DeviceUtils::getBlockDeviceId(const QString &deviceDesc)
{
    QString dev(deviceDesc);
    if (dev.startsWith("/dev/"))
        dev.remove("/dev/");
    return kBlockDeviceIdPrefix + dev;
}

QString DeviceUtils::getMountPointOfDevice(const QString &devPath)
{
    libmnt_table *tab { mnt_new_table() };
    if (!tab)
        return {};
    FinallyUtil finally { [tab]() { if (tab) mnt_free_table(tab); } };
    if (mnt_table_parse_mtab(tab, nullptr) != 0) {
        qWarning() << "Invalid mnt_table_parse_mtab call";
        return {};
    }

    std::string stdPath { devPath.toStdString() };
    auto fs = mnt_table_find_source(tab, stdPath.c_str(), MNT_ITER_BACKWARD);
    if (fs)
        return { mnt_fs_get_target(fs) };

    qWarning() << "Invalid libmnt_fs*";
    return {};
}

QString DeviceUtils::errMessage(dfmmount::DeviceError err)
{
    return DFMMOUNT::Utils::errorMessage(err);
}

/*!
 * \brief DeviceUtils::convertSuitableDisplayName
 * \param devInfo which is obtained by DeviceManager/DeviceProxyManger
 * \return a suitable device name,
 * if device's idLabel is empty, get the display name by size (if size is not 0) or Empty XXX disc (if it is empty disc)
 * and this function should never returns an empty string, if that happened, please check your input.
 */
QString DeviceUtils::convertSuitableDisplayName(const QVariantMap &devInfo)
{
    if (devInfo.value(kHintSystem).toBool()) {
        return nameOfSystemDisk(devInfo);
    } else if (devInfo.value(kIsEncrypted).toBool()) {
        return nameOfEncrypted(devInfo);
    } else if (devInfo.value(kOpticalDrive).toBool()) {
        return nameOfOptical(devInfo);
    } else {
        const QString &&label = devInfo.value(kIdLabel).toString();
        quint64 size = devInfo.value(kSizeTotal).toULongLong();
        return nameOfDefault(label, size);
    }
}

QString DeviceUtils::convertSuitableDisplayName(const QVariantHash &devInfo)
{
    QVariantMap map;
    for (auto iter = devInfo.cbegin(); iter != devInfo.cend(); ++iter)
        map.insert(iter.key(), iter.value());
    return convertSuitableDisplayName(map);
}

bool DeviceUtils::isAutoMountEnable()
{
    return Application::genericAttribute(Application::GenericAttribute::kAutoMount).toBool();
}

bool DeviceUtils::isAutoMountAndOpenEnable()
{
    return Application::genericAttribute(Application::GenericAttribute::kAutoMountAndOpen).toBool();
}

bool DeviceUtils::isWorkingOpticalDiscDev(const QString &dev)
{
    static constexpr char kBurnStateGroup[] { "BurnState" };
    static constexpr char kWoringKey[] { "Working" };

    if (dev.isEmpty())
        return false;

    if (Application::dataPersistence()->keys(kBurnStateGroup).contains(dev)) {
        const QMap<QString, QVariant> &info = Application::dataPersistence()->value(kBurnStateGroup, dev).toMap();
        return info.value(kWoringKey).toBool();
    }
    return false;
}

bool DeviceUtils::isSamba(const QUrl &url)
{
    static const QString smbMatch { "(^/run/user/\\d+/gvfs/smb|^/root/\\.gvfs/smb|^/media/[\\s\\S]*/smbmounts)" };   // TODO(xust) /media/$USER/smbmounts might be changed in the future.}
    return hasMatch(url.path(), smbMatch);
}

bool DeviceUtils::isFtp(const QUrl &url)
{
    static const QString smbMatch { "(^/run/user/\\d+/gvfs/s?ftp|^/root/\\.gvfs/s?ftp)" };
    return hasMatch(url.path(), smbMatch);
}

QString DeviceUtils::nameOfSystemDisk(const QVariantMap &datas)
{
    const auto &lst = Application::genericSetting()->value(BlockAdditionalProperty::kAliasGroupName, BlockAdditionalProperty::kAliasItemName).toList();

    for (const QVariant &v : lst) {
        const QVariantMap &map = v.toMap();
        if (map.value(BlockAdditionalProperty::kAliasItemUUID).toString() == datas.value(kUUID).toString()) {
            return map.value(BlockAdditionalProperty::kAliasItemAlias).toString();
        }
    }

    QString label = datas.value(kIdLabel).toString();
    qlonglong size = datas.value(kSizeTotal).toLongLong();

    // get system disk name if there is no alias
    if (datas.value(kMountPoint).toString() == "/")
        return QObject::tr("System Disk");
    if (datas.value(kIdLabel).toString().startsWith("_dde_"))
        return QObject::tr("Data Disk");
    return nameOfDefault(label, size);
}

QString DeviceUtils::nameOfOptical(const QVariantMap &datas)
{
    QString label = datas.value(kIdLabel).toString();
    if (!label.isEmpty())
        return label;

    static const std::initializer_list<std::pair<QString, QString>> opticalMedias {
        { "optical", "Optical" },
        { "optical_cd", "CD-ROM" },
        { "optical_cd_r", "CD-R" },
        { "optical_cd_rw", "CD-RW" },
        { "optical_dvd", "DVD-ROM" },
        { "optical_dvd_r", "DVD-R" },
        { "optical_dvd_rw", "DVD-RW" },
        { "optical_dvd_ram", "DVD-RAM" },
        { "optical_dvd_plus_r", "DVD+R" },
        { "optical_dvd_plus_rw", "DVD+RW" },
        { "optical_dvd_plus_r_dl", "DVD+R/DL" },
        { "optical_dvd_plus_rw_dl", "DVD+RW/DL" },
        { "optical_bd", "BD-ROM" },
        { "optical_bd_r", "BD-R" },
        { "optical_bd_re", "BD-RE" },
        { "optical_hddvd", "HD DVD-ROM" },
        { "optical_hddvd_r", "HD DVD-R" },
        { "optical_hddvd_rw", "HD DVD-RW" },
        { "optical_mo", "MO" }
    };
    static const QMap<QString, QString> discMapper(opticalMedias);
    static const QVector<std::pair<QString, QString>> discVector(opticalMedias);

    auto totalSize { datas.value(kSizeTotal).toULongLong() };

    if (datas.value(kOptical).toBool()) {   // medium loaded
        if (datas.value(kOpticalBlank).toBool()) {   // show empty disc name
            QString mediaType = datas.value(kMedia).toString();
            return QObject::tr("Blank %1 Disc").arg(discMapper.value(mediaType, QObject::tr("Unknown")));
        } else {
            // totalSize changed after disc mounted
            auto udiks2Size { datas.value(kUDisks2Size).toULongLong() };
            return nameOfDefault(label, udiks2Size > 0 ? udiks2Size : totalSize);
        }
    } else {   // show drive name, medium is not loaded
        auto medias = datas.value(kMediaCompatibility).toStringList();
        QString maxCompatibility;
        for (auto iter = discVector.crbegin(); iter != discVector.crend(); ++iter) {
            if (medias.contains(iter->first))
                return QObject::tr("%1 Drive").arg(iter->second);
        }
    }

    return nameOfDefault(label, totalSize);
}

QString DeviceUtils::nameOfEncrypted(const QVariantMap &datas)
{
    if (datas.value(kCleartextDevice).toString().length() > 1
        && !datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap().isEmpty()) {
        auto clearDevData = datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap();
        QString clearDevLabel = clearDevData.value(kIdLabel).toString();
        qlonglong clearDevSize = clearDevData.value(kSizeTotal).toLongLong();
        return nameOfDefault(clearDevLabel, clearDevSize);
    } else {
        return QObject::tr("%1 Encrypted").arg(nameOfSize(datas.value(kSizeTotal).toLongLong()));
    }
}

QString DeviceUtils::nameOfDefault(const QString &label, const quint64 &size)
{
    if (label.isEmpty())
        return QObject::tr("%1 Volume").arg(nameOfSize(size));
    return label;
}

/*!
 * \brief DeviceUtils::nameOfSize
 * \param size
 * \return
 * infact this function is basically the same as formatSize in FileUtils, but I don't want import any other
 * dfm-base files except device*, so I make a simple copy here.
 */
QString DeviceUtils::nameOfSize(const quint64 &size)
{
    quint64 num = size;
    if (num < 0) {
        qWarning() << "Negative number passed to formatSize():" << num;
        num = 0;
    }

    QStringList list;
    qreal fileSize(num);

    list << " B"
         << " KB"
         << " MB"
         << " GB"
         << " TB";   // should we use KiB since we use 1024 here?

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    return QString("%1 %2").arg(QString::number(fileSize, 'f', 1)).arg(unit);
}

bool DeviceUtils::hasMatch(const QString &txt, const QString &rex)
{
    QRegularExpression re(rex);
    QRegularExpressionMatch match = re.match(txt);
    return match.hasMatch();
}
