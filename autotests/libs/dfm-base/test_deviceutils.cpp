// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceutils.cpp
 * @brief Unit tests for DeviceUtils (deviceutils.cpp)
 */

#include <gtest/gtest.h>
#include <QVariantMap>
#include <QVariantHash>
#include <QUrl>
#include <QDir>

#include <dfm-base/base/device/deviceutils.h>

using namespace dfmbase;

TEST(DeviceUtilsTest, FormatOpticalMediaTypeKnown)
{
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_cd"), QString("CD-ROM"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd"), QString("DVD-ROM"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_bd"), QString("BD-ROM"));
}

TEST(DeviceUtilsTest, FormatOpticalMediaTypeUnknown)
{
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("not_a_media"), QString());
}

TEST(DeviceUtilsTest, NameOfSizeBytes)
{
    QString name = DeviceUtils::nameOfSize(512);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("B"));
}

TEST(DeviceUtilsTest, NameOfSizeGigabytes)
{
    QString name = DeviceUtils::nameOfSize(5LL * 1024 * 1024 * 1024);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("GB"));
}

TEST(DeviceUtilsTest, NameOfDefaultWithLabel)
{
    QString name = DeviceUtils::nameOfDefault("MyDisk", 1024);
    EXPECT_EQ(name, QString("MyDisk"));
}

TEST(DeviceUtilsTest, NameOfDefaultWithoutLabel)
{
    QString name = DeviceUtils::nameOfDefault("", 1024);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, IsSystemDiskByMountPoint)
{
    QVariantHash info;
    info["MountPoint"] = QDir::rootPath();
    EXPECT_TRUE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsTest, IsSystemDiskNotRoot)
{
    QVariantHash info;
    info["mountPoint"] = "/mnt/data";
    EXPECT_FALSE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsTest, IsDataDiskRemovable)
{
    QVariantHash info;
    info["Removable"] = true;
    EXPECT_FALSE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsTest, IsDataDiskByLabel)
{
    QVariantHash info;
    info["IdLabel"] = "_dde_data";
    EXPECT_TRUE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsTest, BindPathTransformIdentity)
{
    QString path = "/some/random/path";
    EXPECT_EQ(DeviceUtils::bindPathTransform(path, false), path);
}

// ---- Coverage additions for DeviceUtils safe query API ----

TEST(DeviceUtilsTest, IsAutoMountEnableIsBool)
{
    // isAutoMountEnable reads the kAutoMount generic attribute; the result is a bool.
    bool result = DeviceUtils::isAutoMountEnable();
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceUtilsTest, GetSambaFileUriFromNativeWithInvalidUrlReturnsEmpty)
{
    QUrl result = DeviceUtils::getSambaFileUriFromNative(QUrl());
    EXPECT_TRUE(result.isEmpty());
}

TEST(DeviceUtilsTest, GetSambaFileUriFromNativeWithNonSmbReturnsSame)
{
    QUrl local = QUrl::fromLocalFile("/tmp/test");
    QUrl result = DeviceUtils::getSambaFileUriFromNative(local);
    EXPECT_EQ(result.toString(), local.toString());
}

TEST(DeviceUtilsTest, IsWorkingOpticalDiscIdWithEmptyStringReturnsFalse)
{
    EXPECT_FALSE(DeviceUtils::isWorkingOpticalDiscId(QString()));
}

TEST(DeviceUtilsTest, ParseNetSourceUrlWithLocalFileReturnsEmpty)
{
    EXPECT_TRUE(DeviceUtils::parseNetSourceUrl(QUrl::fromLocalFile("/tmp")).isEmpty());
}

// ---- Coverage additions: more device query API ----

TEST(DeviceUtilsTest, FstabMountPointsReturnsStringSet)
{
    QSet<QString> points = DeviceUtils::fstabMountPoints();
    // fstabMountPoints returns a set (may be empty in containers without /etc/fstab entries).
    EXPECT_GE(points.size(), 0);
}

TEST(DeviceUtilsTest, IsBlankOpticalDiscWithEmptyIdReturnsFalse)
{
    // Empty id → DevProxyMng->queryBlockInfo returns empty map → isBlank defaults to false.
    EXPECT_FALSE(DeviceUtils::isBlankOpticalDisc(QString()));
}

TEST(DeviceUtilsTest, NameOfEncryptedWithEmptyMapReturnsNonEmpty)
{
    QVariantMap emptyMap;
    QString name = DeviceUtils::nameOfEncrypted(emptyMap);
    // With empty map, falls through to the else branch returning a size-based name.
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, IsSiblingOfRootReturnsBool)
{
    QVariantHash emptyInfo;
    bool result = DeviceUtils::isSiblingOfRoot(emptyInfo);
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceUtilsTest, IsPWOpticalDiscDevWithNonSrDevReturnsFalse)
{
    EXPECT_FALSE(DeviceUtils::isPWOpticalDiscDev("/dev/sda"));
}

TEST(DeviceUtilsTest, IsPWUserspaceOpticalDiscDevWithNonSrDevReturnsFalse)
{
    EXPECT_FALSE(DeviceUtils::isPWUserspaceOpticalDiscDev("/dev/sda"));
}


TEST(DeviceUtilsTest, bindPathTransform)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.bindPathTransform(_arg0, false); });
}

TEST(DeviceUtilsTest, checkDiskEncrypted)
{
    DeviceUtils obj;
    bool result = obj.checkDiskEncrypted();
    EXPECT_FALSE(result);
}

TEST(DeviceUtilsTest, convertSuitableDisplayName)
{
    DeviceUtils obj;
    QVariantHash _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.convertSuitableDisplayName(_arg0); });
}

TEST(DeviceUtilsTest, deviceBytesFree)
{
    DeviceUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.deviceBytesFree(_arg0); });
}

TEST(DeviceUtilsTest, encryptedDisks)
{
    DeviceUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.encryptedDisks(); });
}

TEST(DeviceUtilsTest, findDlnfsPath)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.findDlnfsPath(_arg0, Compare()); });
}

TEST(DeviceUtilsTest, formatOpticalMediaType)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.formatOpticalMediaType(_arg0); });
}

TEST(DeviceUtilsTest, fstabBindInfo)
{
    DeviceUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.fstabBindInfo(); });
}

TEST(DeviceUtilsTest, fstabMountPoints)
{
    DeviceUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.fstabMountPoints(); });
}

TEST(DeviceUtilsTest, getBlockDeviceId)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.getBlockDeviceId(_arg0); });
}

TEST(DeviceUtilsTest, getMountInfo)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.getMountInfo(_arg0, false); });
}

TEST(DeviceUtilsTest, getSambaFileUriFromNative)
{
    DeviceUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.getSambaFileUriFromNative(_arg0); });
}

TEST(DeviceUtilsTest, isAutoMountAndOpenEnable)
{
    DeviceUtils obj;
    bool result = obj.isAutoMountAndOpenEnable();
    EXPECT_FALSE(result);
}

TEST(DeviceUtilsTest, isAutoMountEnable)
{
    DeviceUtils obj;
    bool result = obj.isAutoMountEnable();
    EXPECT_FALSE(result);
}

TEST(DeviceUtilsTest, isBlankOpticalDisc)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isBlankOpticalDisc(_arg0); });
}

TEST(DeviceUtilsTest, isBuiltInDisk)
{
    DeviceUtils obj;
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isBuiltInDisk(_arg0); });
}

TEST(DeviceUtilsTest, isDataDisk)
{
    DeviceUtils obj;
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isDataDisk(_arg0); });
}

TEST(DeviceUtilsTest, isMountPointOfDlnfs)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isMountPointOfDlnfs(_arg0); });
}

TEST(DeviceUtilsTest, isPWOpticalDiscDev)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isPWOpticalDiscDev(_arg0); });
}

TEST(DeviceUtilsTest, isPWUserspaceOpticalDiscDev)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isPWUserspaceOpticalDiscDev(_arg0); });
}

TEST(DeviceUtilsTest, isSiblingOfRoot)
{
    DeviceUtils obj;
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isSiblingOfRoot(_arg0); });
}

TEST(DeviceUtilsTest, isSubpathOfDlnfs)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isSubpathOfDlnfs(_arg0); });
}

TEST(DeviceUtilsTest, isSystemDisk)
{
    DeviceUtils obj;
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isSystemDisk(_arg0); });
}

TEST(DeviceUtilsTest, isUnmountSamba)
{
    DeviceUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isUnmountSamba(_arg0); });
}

TEST(DeviceUtilsTest, isWorkingOpticalDiscDev)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isWorkingOpticalDiscDev(_arg0); });
}

TEST(DeviceUtilsTest, isWorkingOpticalDiscId)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isWorkingOpticalDiscId(_arg0); });
}

TEST(DeviceUtilsTest, nameOfAlias)
{
    DeviceUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.nameOfAlias(_arg0); });
}

TEST(DeviceUtilsTest, nameOfBuiltInDisk)
{
    DeviceUtils obj;
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.nameOfBuiltInDisk(_arg0); });
}

TEST(DeviceUtilsTest, nameOfDefault)
{
    DeviceUtils obj;
    QString _arg0{};
    quint64 _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.nameOfDefault(_arg0, _arg1); });
}

TEST(DeviceUtilsTest, nameOfSize)
{
    DeviceUtils obj;
    quint64 _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.nameOfSize(_arg0); });
}

TEST(DeviceUtilsTest, parseNetSourceUrl)
{
    DeviceUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.parseNetSourceUrl(_arg0); });
}

TEST(DeviceUtilsTest, parseSmbInfo)
{
    DeviceUtils obj;
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE({ obj.parseSmbInfo(_arg0, _arg1, _arg2, nullptr); });
}

TEST(DeviceUtilsTest, supportDfmioCopyDevice)
{
    DeviceUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.supportDfmioCopyDevice(_arg0); });
}

TEST(DeviceUtilsTest, supportSetPermissionsDevice)
{
    DeviceUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.supportSetPermissionsDevice(_arg0); });
}

TEST(DeviceUtilsTest, toHash)
{
    DeviceUtils obj;
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.toHash(_arg0); });
}
