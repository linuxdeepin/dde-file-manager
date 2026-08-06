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
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceUtils::isAutoMountEnable(); });
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

TEST(DeviceUtilsTest, FstabMountPointsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceUtils::fstabMountPoints(); });
}

TEST(DeviceUtilsTest, IsBlankOpticalDiscWithEmptyIdCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceUtils::isBlankOpticalDisc(QString()); });
}

TEST(DeviceUtilsTest, NameOfEncryptedWithEmptyMapCallable)
{
    QVariantMap emptyMap;
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceUtils::nameOfEncrypted(emptyMap); });
}

TEST(DeviceUtilsTest, IsSiblingOfRootCallable)
{
    QVariantHash emptyInfo;
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceUtils::isSiblingOfRoot(emptyInfo); });
}

TEST(DeviceUtilsTest, IsPWOpticalDiscDevWithNonSrDevReturnsFalse)
{
    EXPECT_FALSE(DeviceUtils::isPWOpticalDiscDev("/dev/sda"));
}

TEST(DeviceUtilsTest, IsPWUserspaceOpticalDiscDevWithNonSrDevReturnsFalse)
{
    EXPECT_FALSE(DeviceUtils::isPWUserspaceOpticalDiscDev("/dev/sda"));
}
