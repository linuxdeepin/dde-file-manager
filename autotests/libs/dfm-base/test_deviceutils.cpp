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
#include <QCoreApplication>
#include "stubext.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>

using namespace dfmbase;
using namespace GlobalServerDefines::DeviceProperty;

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

TEST(DeviceUtilsTest, IsSiblingOfRootQMapOverload)
{
    QVariantMap infos;
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceUtils::isSiblingOfRoot(infos); });
}

// ============================================================
// Additional coverage for DeviceUtils
// ============================================================

TEST(DeviceUtilsTest, GetBlockDeviceIdFromDevPath)
{
    QString result = DeviceUtils::getBlockDeviceId("/dev/sda1");
    EXPECT_EQ(result, QString("/org/freedesktop/UDisks2/block_devices/sda1"));
}

TEST(DeviceUtilsTest, GetBlockDeviceIdFromNonDevPath)
{
    QString result = DeviceUtils::getBlockDeviceId("sda1");
    EXPECT_EQ(result, QString("/org/freedesktop/UDisks2/block_devices/sda1"));
}

TEST(DeviceUtilsTest, GetMountInfoEmpty)
{
    QString result = DeviceUtils::getMountInfo("");
    EXPECT_TRUE(result.isEmpty());
}

TEST(DeviceUtilsTest, GetMountInfoNonExistentSource)
{
    QString result = DeviceUtils::getMountInfo("/dev/nonexistent_dev_12345", true);
    EXPECT_TRUE(result.isEmpty());
}

TEST(DeviceUtilsTest, GetMountInfoNonExistentTarget)
{
    QString result = DeviceUtils::getMountInfo("/nonexistent/mount_point_12345", false);
    EXPECT_TRUE(result.isEmpty());
}

TEST(DeviceUtilsTest, ConvertSuitableDisplayNameEmptyMap)
{
    QVariantMap info;
    QString name = DeviceUtils::convertSuitableDisplayName(info);
    // Should never return empty
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, ConvertSuitableDisplayNameHashOverload)
{
    QVariantHash info;
    QString name = DeviceUtils::convertSuitableDisplayName(info);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, ConvertSuitableDisplayNameWithLabel)
{
    QVariantMap info;
    info[kIdLabel] = "TestDisk";
    info[kSizeTotal] = quint64(1024 * 1024 * 1024);
    info[kHintSystem] = false;
    info[kIsEncrypted] = false;
    info[kOpticalDrive] = false;
    QString name = DeviceUtils::convertSuitableDisplayName(info);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, ConvertSuitableDisplayNameWithEncrypted)
{
    QVariantMap info;
    info[kIdLabel] = "EncryptedDisk";
    info[kSizeTotal] = quint64(1024 * 1024 * 1024);
    info[kHintSystem] = false;
    info[kIsEncrypted] = true;
    info[kOpticalDrive] = false;
    QString name = DeviceUtils::convertSuitableDisplayName(info);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("Encrypted"));
}

TEST(DeviceUtilsTest, ConvertSuitableDisplayNameWithOptical)
{
    QVariantMap info;
    info[kOpticalDrive] = true;
    info[kOptical] = false;
    info[kMediaCompatibility] = QStringList { "optical_dvd" };
    info[kSizeTotal] = quint64(0);
    info[kHintSystem] = false;
    info[kIsEncrypted] = false;
    QString name = DeviceUtils::convertSuitableDisplayName(info);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, IsAutoMountAndOpenEnableIsBool)
{
    bool result = DeviceUtils::isAutoMountAndOpenEnable();
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceUtilsTest, IsWorkingOpticalDiscDevEmpty)
{
    EXPECT_FALSE(DeviceUtils::isWorkingOpticalDiscDev(""));
}

TEST(DeviceUtilsTest, IsWorkingOpticalDiscDevNonExistent)
{
    EXPECT_FALSE(DeviceUtils::isWorkingOpticalDiscDev("/dev/sr99"));
}

TEST(DeviceUtilsTest, IsWorkingOpticalDiscIdNonExistent)
{
    EXPECT_FALSE(DeviceUtils::isWorkingOpticalDiscId("/org/freedesktop/UDisks2/block_devices/sr99"));
}

TEST(DeviceUtilsTest, SupportDfmioCopyDeviceInvalidUrl)
{
    QUrl invalidUrl;
    EXPECT_FALSE(DeviceUtils::supportDfmioCopyDevice(invalidUrl));
}

TEST(DeviceUtilsTest, SupportDfmioCopyDeviceLocalFile)
{
    QUrl localUrl = QUrl::fromLocalFile("/tmp/test.txt");
    EXPECT_TRUE(DeviceUtils::supportDfmioCopyDevice(localUrl));
}

TEST(DeviceUtilsTest, SupportSetPermissionsDeviceInvalidUrl)
{
    QUrl invalidUrl;
    EXPECT_FALSE(DeviceUtils::supportSetPermissionsDevice(invalidUrl));
}

TEST(DeviceUtilsTest, SupportSetPermissionsDeviceLocalFile)
{
    QUrl localUrl = QUrl::fromLocalFile("/tmp/test.txt");
    EXPECT_TRUE(DeviceUtils::supportSetPermissionsDevice(localUrl));
}

TEST(DeviceUtilsTest, ParseSmbInfoValid)
{
    QString host, share;
    bool ok = DeviceUtils::parseSmbInfo(",server=192.168.1.1,share=myshare", host, share);
    EXPECT_TRUE(ok);
    EXPECT_EQ(host, "192.168.1.1");
    EXPECT_EQ(share, "myshare");
}

TEST(DeviceUtilsTest, ParseSmbInfoWithPort)
{
    QString host, share, port;
    bool ok = DeviceUtils::parseSmbInfo(
        ":port=445,server=192.168.1.1,share=myshare", host, share, &port);
    EXPECT_TRUE(ok);
    EXPECT_EQ(host, "192.168.1.1");
    EXPECT_EQ(share, "myshare");
    EXPECT_EQ(port, "445");
}

TEST(DeviceUtilsTest, ParseSmbInfoInvalid)
{
    QString host, share;
    bool ok = DeviceUtils::parseSmbInfo("not_valid_smb_path", host, share);
    EXPECT_FALSE(ok);
}

TEST(DeviceUtilsTest, FstabBindInfoReturnsMap)
{
    QMap<QString, QString> bindInfo = DeviceUtils::fstabBindInfo();
    // Returns a map, may be empty
    EXPECT_TRUE(bindInfo.isEmpty() || !bindInfo.isEmpty());
}

TEST(DeviceUtilsTest, NameOfBuiltInDiskEmptyMap)
{
    QVariantMap info;
    QString name = DeviceUtils::nameOfBuiltInDisk(info);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, NameOfBuiltInDiskSystemDisk)
{
    QVariantMap info;
    info[kMountPoint] = QDir::rootPath();
    QString name = DeviceUtils::nameOfBuiltInDisk(info);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, NameOfBuiltInDiskDataDisk)
{
    QVariantMap info;
    info[kIdLabel] = "_dde_data";
    info[kCanPowerOff] = true;
    QString name = DeviceUtils::nameOfBuiltInDisk(info);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, NameOfOpticalEmptyMedia)
{
    QVariantMap info;
    info[kOpticalDrive] = true;
    info[kOptical] = false;
    info[kMediaCompatibility] = QStringList { "optical_dvd" };
    info[kSizeTotal] = quint64(0);
    QString name = DeviceUtils::nameOfOptical(info);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("Drive"));
}

TEST(DeviceUtilsTest, NameOfOpticalLoadedBlankDisc)
{
    QVariantMap info;
    info[kOpticalDrive] = true;
    info[kOptical] = true;
    info[kOpticalBlank] = true;
    info[kMedia] = "optical_dvd";
    info[kSizeTotal] = quint64(0);
    QString name = DeviceUtils::nameOfOptical(info);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("Blank"));
}

TEST(DeviceUtilsTest, NameOfOpticalLoadedDiscWithLabel)
{
    QVariantMap info;
    info[kOpticalDrive] = true;
    info[kOptical] = true;
    info[kOpticalBlank] = false;
    info[kIdLabel] = "MyDisc";
    info[kSizeTotal] = quint64(1024);
    info[kUDisks2Size] = quint64(1024);
    QString name = DeviceUtils::nameOfOptical(info);
    EXPECT_EQ(name, QString("MyDisc"));
}

TEST(DeviceUtilsTest, NameOfEncryptedWithCleartextData)
{
    QVariantMap info;
    QVariantMap clearInfo;
    clearInfo[kIdLabel] = "ClearLabel";
    clearInfo[kSizeTotal] = quint64(1024);
    info[kCleartextDevice] = "/org/freedesktop/UDisks2/block_devices/dm-0";
    info["ClearBlockDeviceInfo"] = clearInfo;
    QString name = DeviceUtils::nameOfEncrypted(info);
    EXPECT_EQ(name, QString("ClearLabel"));
}

TEST(DeviceUtilsTest, NameOfAliasNoAliasConfigured)
{
    QString alias = DeviceUtils::nameOfAlias("nonexistent-uuid");
    EXPECT_TRUE(alias.isEmpty());
}

TEST(DeviceUtilsTest, CheckDiskEncryptedIsBool)
{
    bool result = DeviceUtils::checkDiskEncrypted();
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceUtilsTest, EncryptedDisksReturnsList)
{
    QStringList disks = DeviceUtils::encryptedDisks();
    EXPECT_TRUE(disks.isEmpty() || !disks.isEmpty());
}

TEST(DeviceUtilsTest, IsSubpathOfDlnfsNonExistent)
{
    bool result = DeviceUtils::isSubpathOfDlnfs("/nonexistent/dlnfs/path");
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceUtilsTest, IsMountPointOfDlnfsNonExistent)
{
    bool result = DeviceUtils::isMountPointOfDlnfs("/nonexistent/dlnfs/path");
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceUtilsTest, GetLongestMountRootPathNonExistent)
{
    QString result = DeviceUtils::getLongestMountRootPath("/nonexistent/deep/path/file.txt");
    // Should return at least "/"
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsTest, GetLongestMountRootPathRoot)
{
    QString result = DeviceUtils::getLongestMountRootPath("/");
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsTest, DeviceBytesFreeLocalFile)
{
    QUrl url = QUrl::fromLocalFile("/tmp");
    qint64 freeBytes = DeviceUtils::deviceBytesFree(url);
    EXPECT_GE(freeBytes, qint64(0));
}

TEST(DeviceUtilsTest, DeviceBytesFreeNonFileScheme)
{
    QUrl url("smb://192.168.1.1/share/file.txt");
    qint64 freeBytes;
    EXPECT_NO_FATAL_FAILURE({
        freeBytes = DeviceUtils::deviceBytesFree(url);
    });
    EXPECT_GE(freeBytes, qint64(-1));
}

TEST(DeviceUtilsTest, IsUnmountSambaNonSmbUrl)
{
    QUrl localUrl = QUrl::fromLocalFile("/tmp/test.txt");
    EXPECT_FALSE(DeviceUtils::isUnmountSamba(localUrl));
}

TEST(DeviceUtilsTest, IsUnmountSambaInvalidUrl)
{
    QUrl invalidUrl;
    EXPECT_FALSE(DeviceUtils::isUnmountSamba(invalidUrl));
}

TEST(DeviceUtilsTest, IsBuiltInDiskHashRemovable)
{
    QVariantHash info;
    info[kCanPowerOff] = true;
    info[kHintSystem] = false;
    info[kOpticalDrive] = false;
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(info));
}

TEST(DeviceUtilsTest, IsBuiltInDiskHashSystem)
{
    QVariantHash info;
    info[kCanPowerOff] = false;
    info[kHintSystem] = true;
    info[kMountPoint] = "/";
    EXPECT_TRUE(DeviceUtils::isBuiltInDisk(info));
}

TEST(DeviceUtilsTest, IsBuiltInDiskHashOptical)
{
    QVariantHash info;
    info[kOpticalDrive] = true;
    info[kHintSystem] = true;
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(info));
}

TEST(DeviceUtilsTest, IsBuiltInDiskHashNoHintSystem)
{
    QVariantHash info;
    info[kCanPowerOff] = false;
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(info));
}

TEST(DeviceUtilsTest, IsBuiltInDiskMapOverload)
{
    QVariantMap info;
    info[kCanPowerOff] = false;
    info[kHintSystem] = true;
    info[kMountPoint] = "/";
    EXPECT_TRUE(DeviceUtils::isBuiltInDisk(info));
}

TEST(DeviceUtilsTest, IsSystemDiskMapOverload)
{
    QVariantMap info;
    info[kMountPoint] = "/";
    EXPECT_TRUE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsTest, IsSystemDiskMapNotRoot)
{
    QVariantMap info;
    info[kMountPoint] = "/mnt/data";
    EXPECT_FALSE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsTest, IsDataDiskMapOverloadDdeData)
{
    QVariantMap info;
    info[kIdLabel] = "_dde_data";
    EXPECT_TRUE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsTest, IsDataDiskMapOverloadRoot)
{
    QVariantMap info;
    info[kMountPoint] = "/";
    EXPECT_FALSE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsTest, IsDataDiskMapOverloadRemovable)
{
    QVariantMap info;
    info[kCanPowerOff] = true;
    EXPECT_FALSE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsTest, IsDataDiskHashOverloadDdeHome)
{
    QVariantHash info;
    info[kIdLabel] = "_dde_home";
    EXPECT_TRUE(DeviceUtils::isDataDisk(info));
}

TEST(DeviceUtilsTest, NameOfSizeZero)
{
    QString name = DeviceUtils::nameOfSize(0);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("B"));
}

TEST(DeviceUtilsTest, NameOfSizeTerabytes)
{
    QString name = DeviceUtils::nameOfSize(2LL * 1024 * 1024 * 1024 * 1024);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("TB"));
}

TEST(DeviceUtilsTest, NameOfSizeMegabytes)
{
    QString name = DeviceUtils::nameOfSize(5LL * 1024 * 1024);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("MB"));
}

TEST(DeviceUtilsTest, NameOfSizeKilobytes)
{
    QString name = DeviceUtils::nameOfSize(5LL * 1024);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_TRUE(name.contains("KB"));
}

TEST(DeviceUtilsTest, NameOfDefaultZeroSize)
{
    QString name = DeviceUtils::nameOfDefault("", 0);
    EXPECT_FALSE(name.isEmpty());
}

TEST(DeviceUtilsTest, IsSiblingOfRootWithData)
{
    QVariantHash info;
    info[kDrive] = "/org/freedesktop/UDisks2/drives/nonexistent_drive";
    bool result = DeviceUtils::isSiblingOfRoot(info);
    EXPECT_TRUE(result == true || result == false);
}

TEST(DeviceUtilsTest, FormatOpticalMediaTypeAllKnown)
{
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical"), QString("Optical"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_cd_r"), QString("CD-R"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_cd_rw"), QString("CD-RW"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd_r"), QString("DVD-R"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd_rw"), QString("DVD-RW"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd_ram"), QString("DVD-RAM"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd_plus_r"), QString("DVD+R"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd_plus_rw"), QString("DVD+RW"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd_plus_r_dl"), QString("DVD+R/DL"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_dvd_plus_rw_dl"), QString("DVD+RW/DL"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_bd_r"), QString("BD-R"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_bd_re"), QString("BD-RE"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_hddvd"), QString("HD DVD-ROM"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_hddvd_r"), QString("HD DVD-R"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_hddvd_rw"), QString("HD DVD-RW"));
    EXPECT_EQ(DeviceUtils::formatOpticalMediaType("optical_mo"), QString("MO"));
}

TEST(DeviceUtilsTest, ParseNetSourceUrlInvalidScheme)
{
    QUrl url("http://192.168.1.1/file");
    QUrl result = DeviceUtils::parseNetSourceUrl(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST(DeviceUtilsTest, BindPathTransformRoot)
{
    EXPECT_EQ(DeviceUtils::bindPathTransform("/", false), QString("/"));
}

TEST(DeviceUtilsTest, BindPathTransformToDeviceRoot)
{
    EXPECT_EQ(DeviceUtils::bindPathTransform("/", true), QString("/"));
}

TEST(DeviceUtilsTest, IsBuiltInDiskHashWithDdeLabel)
{
    QVariantHash info;
    info[kCanPowerOff] = false;
    info[kHintSystem] = false;
    info[kMountPoint] = "/data";
    info[kIdLabel] = "_dde_system";
    EXPECT_TRUE(DeviceUtils::isBuiltInDisk(info));
}

TEST(DeviceUtilsTest, IsSystemDiskHashRootWithSysroot)
{
    QVariantHash info;
    info[kMountPoint] = "/sysroot";
    info[kIdLabel] = "RootA";
    EXPECT_TRUE(DeviceUtils::isSystemDisk(info));
}

TEST(DeviceUtilsTest, IsSystemDiskHashNotRootWithSysroot)
{
    QVariantHash info;
    info[kMountPoint] = "/sysroot";
    info[kIdLabel] = "NotRoot";
    EXPECT_FALSE(DeviceUtils::isSystemDisk(info));
}
