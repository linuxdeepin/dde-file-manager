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
#ifndef DEVICEUTILS_H
#define DEVICEUTILS_H

#include "dfm-base/dfm_base_global.h"

#include <QString>

#include <dfm-mount/base/dmountutils.h>

namespace dfmbase {

namespace BlockAdditionalProperty {
inline constexpr char kClearBlockProperty[] { "ClearBlockDeviceInfo" };
inline constexpr char kAliasGroupName[] { "LocalDiskAlias" };
inline constexpr char kAliasItemName[] { "Items" };
inline constexpr char kAliasItemUUID[] { "uuid" };
inline constexpr char kAliasItemAlias[] { "alias" };
}   // namespace BlockAdditionalProperty

inline constexpr char kBlockDeviceIdPrefix[] { "/org/freedesktop/UDisks2/block_devices/" };

/*!
 * \brief The DeviceUtils class
 * this class provide some util functions.
 */
class DeviceUtils
{
public:
    static QString getBlockDeviceId(const QString &deviceDesc);
    static QString getMountInfo(const QString &in, bool lookForMpt = true);
    static QString errMessage(DFMMOUNT::DeviceError err);
    static QString convertSuitableDisplayName(const QVariantMap &devInfo);
    static QString convertSuitableDisplayName(const QVariantHash &devInfo);
    static QString formatOpticalMediaType(const QString &media);
    static bool isAutoMountEnable();
    static bool isAutoMountAndOpenEnable();
    static bool isWorkingOpticalDiscDev(const QString &dev);
    static bool isWorkingOpticalDiscId(const QString &id);

    static bool isSamba(const QUrl &url);
    static bool isFtp(const QUrl &url);

    static QMap<QString, QString> fstabBindInfo();

private:
    static QString nameOfSystemDisk(const QVariantMap &datas);
    static QString nameOfOptical(const QVariantMap &datas);
    static QString nameOfEncrypted(const QVariantMap &datas);
    static QString nameOfDefault(const QString &label, const quint64 &size);
    static QString nameOfSize(const quint64 &size);

    static bool hasMatch(const QString &txt, const QString &rex);
};

}

#endif   // DEVICEUTILS_H
