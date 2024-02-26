// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEUTILS_H
#define DEVICEUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QString>

#include <dfm-mount/base/dmountutils.h>

#include <functional>

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
    static QUrl getSambaFileUriFromNative(const QUrl &url);
    static QString errMessage(DFMMOUNT::DeviceError err);
    static QString convertSuitableDisplayName(const QVariantMap &devInfo);
    static QString convertSuitableDisplayName(const QVariantHash &devInfo);
    static QString formatOpticalMediaType(const QString &media);
    static bool isAutoMountEnable();
    static bool isAutoMountAndOpenEnable();

    static bool isWorkingOpticalDiscDev(const QString &dev);
    static bool isWorkingOpticalDiscId(const QString &id);
    static bool isBlankOpticalDisc(const QString &id);
    static bool isPWOpticalDiscDev(const QString &dev);
    static bool isPWUserspaceOpticalDiscDev(const QString &dev);

    static bool isSamba(const QUrl &url);
    static bool isFtp(const QUrl &url);
    static bool isSftp(const QUrl &url);
    static bool isMtpFile(const QUrl &url);
    static bool supportDfmioCopyDevice(const QUrl &url);
    static bool supportSetPermissionsDevice(const QUrl &url);
    static bool isExternalBlock(const QUrl &url);
    static QUrl parseNetSourceUrl(const QUrl &target);

    static bool parseSmbInfo(const QString &smbPath, QString &host, QString &share, QString *port = nullptr);

    static QMap<QString, QString> fstabBindInfo();

    static QString nameOfSystemDisk(const QVariantMap &datas);
    static QString nameOfOptical(const QVariantMap &datas);
    static QString nameOfEncrypted(const QVariantMap &datas);
    static QString nameOfDefault(const QString &label, const quint64 &size);
    static QString nameOfSize(const quint64 &size);
    static QString nameOfAlias(const QString &uuid);

    static bool checkDiskEncrypted();
    static QStringList encryptedDisks();

    static bool isSubpathOfDlnfs(const QString &path);
    static bool isMountPointOfDlnfs(const QString &path);
    static bool isLowSpeedDevice(const QUrl &url);

    static QString getLongestMountRootPath(const QString &filePath);

    static QString fileSystemType(const QUrl &url);
    static qint64 deviceBytesFree(const QUrl &url);
    static bool isUnmountSamba(const QUrl &url);
    // If toDevice is true, convert the path to the device name
    // otherwise convert the path to the mount point name
    static QString bindPathTransform(const QString &path, bool toDevice);

    static bool isSystemDisk(const QVariantHash &devInfo);
    static bool isSystemDisk(const QVariantMap &devInfo);
    static bool isSiblingOfRoot(const QVariantHash &devInfo);
    static bool isSiblingOfRoot(const QVariantMap &devInfo);

private:
    static bool hasMatch(const QString &txt, const QString &rex);
    using Compare = std::function<bool(const QString &, const QString &)>;
    static bool findDlnfsPath(const QString &target, Compare func);
};

}

#endif   // DEVICEUTILS_H
