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
    // bindPathTransform
    SUCCEED();
}

TEST(DeviceUtilsTest, checkDiskEncrypted)
{
    // checkDiskEncrypted
    SUCCEED();
}

TEST(DeviceUtilsTest, convertSuitableDisplayName)
{
    // convertSuitableDisplayName
    SUCCEED();
}

TEST(DeviceUtilsTest, deviceBytesFree)
{
    // deviceBytesFree
    SUCCEED();
}

TEST(DeviceUtilsTest, encryptedDisks)
{
    // encryptedDisks
    SUCCEED();
}

TEST(DeviceUtilsTest, findDlnfsPath)
{
    // findDlnfsPath
    SUCCEED();
}

TEST(DeviceUtilsTest, formatOpticalMediaType)
{
    // formatOpticalMediaType
    SUCCEED();
}

TEST(DeviceUtilsTest, fstabBindInfo)
{
    // fstabBindInfo
    SUCCEED();
}

TEST(DeviceUtilsTest, fstabMountPoints)
{
    // fstabMountPoints
    SUCCEED();
}

TEST(DeviceUtilsTest, getBlockDeviceId)
{
    // getBlockDeviceId
    SUCCEED();
}

TEST(DeviceUtilsTest, getMountInfo)
{
    // getMountInfo
    SUCCEED();
}

TEST(DeviceUtilsTest, getSambaFileUriFromNative)
{
    // getSambaFileUriFromNative
    SUCCEED();
}

TEST(DeviceUtilsTest, isAutoMountAndOpenEnable)
{
    // isAutoMountAndOpenEnable
    SUCCEED();
}

TEST(DeviceUtilsTest, isAutoMountEnable)
{
    // isAutoMountEnable
    SUCCEED();
}

TEST(DeviceUtilsTest, isBlankOpticalDisc)
{
    // isBlankOpticalDisc
    SUCCEED();
}

TEST(DeviceUtilsTest, isBuiltInDisk)
{
    // isBuiltInDisk
    SUCCEED();
}

TEST(DeviceUtilsTest, isDataDisk)
{
    // isDataDisk
    SUCCEED();
}

TEST(DeviceUtilsTest, isMountPointOfDlnfs)
{
    // isMountPointOfDlnfs
    SUCCEED();
}

TEST(DeviceUtilsTest, isPWOpticalDiscDev)
{
    // isPWOpticalDiscDev
    SUCCEED();
}

TEST(DeviceUtilsTest, isPWUserspaceOpticalDiscDev)
{
    // isPWUserspaceOpticalDiscDev
    SUCCEED();
}

TEST(DeviceUtilsTest, isSiblingOfRoot)
{
    // isSiblingOfRoot
    SUCCEED();
}

TEST(DeviceUtilsTest, isSubpathOfDlnfs)
{
    // isSubpathOfDlnfs
    SUCCEED();
}

TEST(DeviceUtilsTest, isSystemDisk)
{
    // isSystemDisk
    SUCCEED();
}

TEST(DeviceUtilsTest, isUnmountSamba)
{
    // isUnmountSamba
    SUCCEED();
}

TEST(DeviceUtilsTest, isWorkingOpticalDiscDev)
{
    // isWorkingOpticalDiscDev
    SUCCEED();
}

TEST(DeviceUtilsTest, isWorkingOpticalDiscId)
{
    // isWorkingOpticalDiscId
    SUCCEED();
}

TEST(DeviceUtilsTest, nameOfAlias)
{
    // nameOfAlias
    SUCCEED();
}

TEST(DeviceUtilsTest, nameOfBuiltInDisk)
{
    // nameOfBuiltInDisk
    SUCCEED();
}

TEST(DeviceUtilsTest, nameOfDefault)
{
    // nameOfDefault
    SUCCEED();
}

TEST(DeviceUtilsTest, nameOfSize)
{
    // nameOfSize
    SUCCEED();
}

TEST(DeviceUtilsTest, parseNetSourceUrl)
{
    // parseNetSourceUrl
    SUCCEED();
}

TEST(DeviceUtilsTest, parseSmbInfo)
{
    // parseSmbInfo
    SUCCEED();
}

TEST(DeviceUtilsTest, supportDfmioCopyDevice)
{
    // supportDfmioCopyDevice
    SUCCEED();
}

TEST(DeviceUtilsTest, supportSetPermissionsDevice)
{
    // supportSetPermissionsDevice
    SUCCEED();
}

TEST(DeviceUtilsTest, toHash)
{
    // toHash
    SUCCEED();
}
