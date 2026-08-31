// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceutils_ext2.cpp
 * @brief Second batch of DeviceUtils tests targeting remaining functions.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariantMap>
#include <QVariantHash>
#include <QMap>

#include <dfm-base/base/device/deviceutils.h>

using namespace dfmbase;

TEST(DeviceUtilsExt2Test, NameOfBuiltInDiskVariantMap)
{
    QVariantMap info;
    info.insert("OpticalDrive", true);
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::nameOfBuiltInDisk(info); });
}

TEST(DeviceUtilsExt2Test, NameOfOpticalVariantMap)
{
    QVariantMap info;
    info.insert("IdLabel", "MyDisc");
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::nameOfOptical(info); });
}

TEST(DeviceUtilsExt2Test, NameOfDefaultEmpty)
{
    QString name = DeviceUtils::nameOfDefault("", 1024);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsExt2Test, NameOfSizeVarious)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::nameOfSize(0); });
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::nameOfSize(500ULL * 1024 * 1024); });
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::nameOfSize(2ULL * 1024 * 1024 * 1024); });
}

TEST(DeviceUtilsExt2Test, NameOfAliasEmpty)
{
    EXPECT_TRUE(DeviceUtils::nameOfAlias("definitely-not-a-real-uuid").isEmpty());
}

TEST(DeviceUtilsExt2Test, IsBuiltInDiskVariantMap)
{
    QVariantMap info;
    info.insert("OpticalDrive", true);
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(info));
}

TEST(DeviceUtilsExt2Test, IsSystemDiskVariantMap)
{
    QVariantMap info;
    info.insert("MountPoint", "/data");
    EXPECT_FALSE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsExt2Test, IsSiblingOfRootVariantHash)
{
    QVariantHash info;
    info.insert("Drive", "non-matching-drive");
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::isSiblingOfRoot(info); });
}

TEST(DeviceUtilsExt2Test, IsSubpathOfDlnfs)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::isSubpathOfDlnfs("/some/arbitrary/path"); });
}

TEST(DeviceUtilsExt2Test, IsMountPointOfDlnfs)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::isMountPointOfDlnfs("/some/arbitrary/path"); });
}

TEST(DeviceUtilsExt2Test, GetLongestMountRootPath)
{
    QString r = DeviceUtils::getLongestMountRootPath("/home/user/docs");
    EXPECT_FALSE(r.isEmpty());
}

TEST(DeviceUtilsExt2Test, DeviceBytesFreeInvalidUrl)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::deviceBytesFree(QUrl()); });
}

TEST(DeviceUtilsExt2Test, CheckDiskEncrypted)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::checkDiskEncrypted(); });
}

TEST(DeviceUtilsExt2Test, EncryptedDisks)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::encryptedDisks(); });
}

TEST(DeviceUtilsExt2Test, IsAutoMountAndOpenEnable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::isAutoMountAndOpenEnable(); });
}

TEST(DeviceUtilsExt2Test, IsWorkingOpticalDiscDev)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::isWorkingOpticalDiscDev("/dev/sr0"); });
}

TEST(DeviceUtilsExt2Test, SupportDfmioCopyDevice)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::supportDfmioCopyDevice(QUrl("file:///tmp")); });
}

TEST(DeviceUtilsExt2Test, SupportSetPermissionsDevice)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::supportSetPermissionsDevice(QUrl("file:///tmp")); });
}

TEST(DeviceUtilsExt2Test, ConvertSuitableDisplayNameHashWithClear)
{
    QVariantHash info;
    info.insert("IdLabel", "MyUsb");
    info.insert("SizeTotal", 1024 * 1024 * 100);
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::convertSuitableDisplayName(info); });
}

TEST(DeviceUtilsExt2Test, ConvertSuitableDisplayNameMap)
{
    QVariantMap info;
    info.insert("IdLabel", "MyUsb2");
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::convertSuitableDisplayName(info); });
}

TEST(DeviceUtilsExt2Test, GetMountInfo)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::getMountInfo("/", false); });
}

TEST(DeviceUtilsExt2Test, GetBlockDeviceId)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceUtils::getBlockDeviceId("/dev/sda1"); });
}
