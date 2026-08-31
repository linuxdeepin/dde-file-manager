// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceutils_ext.cpp
 * @brief Extended unit tests for DeviceUtils pure-logic functions.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariantMap>
#include <QVariantHash>
#include <QMap>

#include <dfm-base/base/device/deviceutils.h>

using namespace dfmbase;

TEST(DeviceUtilsExtTest, FormatOpticalMediaTypeKnown)
{
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_cd"), QString("CD-ROM"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd"), QString("DVD-ROM"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_bd"), QString("BD-ROM"));
}

TEST(DeviceUtilsExtTest, FormatOpticalMediaTypeUnknown)
{
    EXPECT_TRUE(DeviceUtils::formatOpticalMediaType("no_such_media").isEmpty());
}

TEST(DeviceUtilsExtTest, ParseSmbInfoBasic)
{
    QString host, share;
    QString port;
    bool ok = DeviceUtils::parseSmbInfo(",server=myhost,share=share1", host, share, &port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(host, QString("myhost"));
    EXPECT_EQ(share, QString("share1"));
}

TEST(DeviceUtilsExtTest, ParseSmbInfoWithPort)
{
    QString host, share;
    QString port;
    bool ok = DeviceUtils::parseSmbInfo(":port=445,server=h,share=s", host, share, &port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(host, QString("h"));
    EXPECT_EQ(share, QString("s"));
    EXPECT_EQ(port, QString("445"));
}

TEST(DeviceUtilsExtTest, ParseSmbInfoNoMatch)
{
    QString host, share;
    bool ok = DeviceUtils::parseSmbInfo("not-a-valid-smb-path", host, share);
    EXPECT_FALSE(ok);
}

TEST(DeviceUtilsExtTest, NameOfDefaultWithLabel)
{
    EXPECT_EQ(DeviceUtils::nameOfDefault("mydisk", 1024), QString("mydisk"));
}

TEST(DeviceUtilsExtTest, NameOfDefaultNoLabel)
{
    QString name = DeviceUtils::nameOfDefault("", 1024);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsExtTest, NameOfSizeZero)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::nameOfSize(0); });
}

TEST(DeviceUtilsExtTest, NameOfSizeLarge)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::nameOfSize(1024ULL * 1024 * 1024 * 500); });
}

TEST(DeviceUtilsExtTest, IsSystemDiskRootMount)
{
    QVariantHash info;
    info.insert("MountPoint", "/");
    EXPECT_TRUE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsExtTest, IsSystemDiskNonRoot)
{
    QVariantHash info;
    info.insert("MountPoint", "/data");
    EXPECT_FALSE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsExtTest, IsSystemDiskRootRecover)
{
    QVariantHash info;
    info.insert("MountPoint", "/sysroot");
    info.insert("IdLabel", "RootA");
    EXPECT_TRUE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsExtTest, IsDataDiskNonRemovable)
{
    QVariantHash info;
    info.insert("MountPoint", "/data");
    info.insert("IdLabel", "_dde_data");
    EXPECT_TRUE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsExtTest, IsDataDiskRootIsNotData)
{
    QVariantHash info;
    info.insert("MountPoint", "/");
    EXPECT_FALSE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsExtTest, IsRemovableDeviceOptical)
{
    QVariantHash info;
    info.insert("CanPowerOff", true);
    info.insert("Drive", "some-drive-xyz");   // differs from (likely empty) root drive
    EXPECT_TRUE(DeviceUtils::isRemovableDevice(info));
}

TEST(DeviceUtilsExtTest, IsRemovableDeviceNotPowerOff)
{
    QVariantHash info;
    // CanPowerOff defaults to false -> not removable
    EXPECT_FALSE(DeviceUtils::isRemovableDevice(info));
}

TEST(DeviceUtilsExtTest, BindPathTransformRootUnchanged)
{
    EXPECT_EQ(DeviceUtils::bindPathTransform("/", true), QString("/"));
    EXPECT_EQ(DeviceUtils::bindPathTransform("relative", true), QString("relative"));
}

TEST(DeviceUtilsExtTest, IsSubpathOfDlnfs)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::isSubpathOfDlnfs("/some/path"); });
}

TEST(DeviceUtilsExtTest, IsMountPointOfDlnfs)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::isMountPointOfDlnfs("/some/path"); });
}

TEST(DeviceUtilsExtTest, GetLongestMountRootPath)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::getLongestMountRootPath("/some/path"); });
}

TEST(DeviceUtilsExtTest, ConvertSuitableDisplayNameVariantHash)
{
    QVariantHash info;
    info.insert("IdLabel", "usb");
    info.insert("SizeTotal", 0);
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::convertSuitableDisplayName(info); });
}

TEST(DeviceUtilsExtTest, NameOfAliasEmpty)
{
    EXPECT_TRUE(DeviceUtils::nameOfAlias("no-such-uuid").isEmpty());
}

TEST(DeviceUtilsExtTest, NameOfBuiltInDiskOpticalIsNot)
{
    QVariantMap info;
    info.insert("OpticalDrive", true);
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(info));
}
