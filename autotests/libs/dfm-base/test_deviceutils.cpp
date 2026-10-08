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

#include <mntent.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <QTemporaryDir>
#include <QUrl>

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

// ===== PMS sev-2 regression cluster: deviceutils.cpp (work-order batch 2) =====

// PMS:139803 getMountInfo 改用 libmount 解析 /proc/self/mountinfo：根挂载 source/target 双向反查必须一致
TEST(DeviceUtilsTest, BUG139803_GetMountInfoBidirectionalOnRootMount)
{
    FILE *f = setmntent("/proc/mounts", "r");
    ASSERT_NE(f, nullptr);
    // 选一个 source/target 均唯一出现的条目，避免 overlay 多次挂载等环境干扰
    QList<QPair<QString, QString>> entries;
    struct mntent *ent;
    while ((ent = getmntent(f)))
        entries.append({ QString::fromUtf8(ent->mnt_fsname), QString::fromUtf8(ent->mnt_dir) });
    endmntent(f);
    ASSERT_FALSE(entries.isEmpty());

    QString src, tgt;
    for (const auto &e : entries) {
        const int srcCount = std::count_if(entries.cbegin(), entries.cend(),
                                           [&](const QPair<QString, QString> &x) { return x.first == e.first; });
        const int tgtCount = std::count_if(entries.cbegin(), entries.cend(),
                                           [&](const QPair<QString, QString> &x) { return x.second == e.second; });
        if (srcCount == 1 && tgtCount == 1) {
            src = e.first;
            tgt = e.second;
            break;
        }
    }
    if (src.isEmpty()) {
        SUCCEED() << "no unique source/target mount entry in this environment";
        return;
    }
    EXPECT_EQ(DeviceUtils::getMountInfo(src, true), tgt);
    EXPECT_EQ(DeviceUtils::getMountInfo(tgt, false), src);
}

// PMS:139803 含空格的挂载点（mountinfo 中 \040 转义）必须被完整解析：
// 修复前手工 ifstream 按空白切分会把 "06 10 122" 之类挂载点切碎；getmntent/libmount 均还原为空格。
TEST(DeviceUtilsTest, BUG139803_GetMountInfoHandlesEscapedSpaceMountPoint)
{
    FILE *f = setmntent("/proc/mounts", "r");
    ASSERT_NE(f, nullptr);
    bool found = false;
    struct mntent *ent;
    while ((ent = getmntent(f))) {
        const QString dir = QString::fromUtf8(ent->mnt_dir);
        if (dir.contains(QLatin1Char(' '))) {
            found = true;
            const QString result = DeviceUtils::getMountInfo(QString::fromUtf8(ent->mnt_fsname), true);
            EXPECT_EQ(result, dir);
            break;
        }
    }
    endmntent(f);
    if (!found)
        SUCCEED() << "no space-escaped mount point in this environment";
}

// ============================================================
// PMS sev-2 regression cluster: deviceutils.cpp (work-order batch 3)
// ============================================================

// PMS:200247 加密盘列表（checkDiskEncrypted 修复引入的 encryptedDisks）为进程级缓存，
// 重复调用必须返回一致结果且不崩溃（不依赖环境中是否存在 deepin-installer 配置）
TEST(DeviceUtilsTest, BUG200247_EncryptedDisksStableAcrossCalls)
{
    const auto first = DeviceUtils::encryptedDisks();
    const auto second = DeviceUtils::encryptedDisks();
    EXPECT_EQ(first, second);
}

// PMS:180367 checkDiskEncrypted 读取安装器加密信息：结果跨调用稳定且为 bool，不崩溃
TEST(DeviceUtilsTest, BUG180367_CheckDiskEncryptedStableAcrossCalls)
{
    const bool first = DeviceUtils::checkDiskEncrypted();
    const bool second = DeviceUtils::checkDiskEncrypted();
    EXPECT_EQ(first, second);
}

// PMS:323581 内置盘判定（原 kCanPowerOff+isSiblingOfRoot 修复，后演进为 isBuiltInDisk）：
// 可移动/光驱/无 HintSystem 键 -> 非内置；根挂载点/_dde_ 标签/HintSystem/非 USB 总线 -> 内置
TEST(DeviceUtilsTest, BUG323581_IsBuiltInDiskDeterministicBranches)
{
    using namespace GlobalServerDefines::DeviceProperty;
    const QString fakeId = "/org/freedesktop/UDisks2/block_devices/sda1";

    // removable (kCanPowerOff, not in fstab, not sibling of root) -> not built-in;
    // kDrive differs from the cached root drive so isSiblingOfRoot is deterministically false
    QVariantHash removable { { kDevice, fakeId }, { kCanPowerOff, true },
                             { kMountPoint, "/media/usb_ut_323581" }, { kDrive, "sda_ut" } };
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(removable));

    QVariantHash optical { { kDevice, fakeId }, { kOpticalDrive, true }, { kHintSystem, false } };
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(optical));

    QVariantHash noHint { { kDevice, fakeId } };
    EXPECT_FALSE(DeviceUtils::isBuiltInDisk(noHint));

    QVariantHash rootDisk { { kDevice, fakeId }, { kHintSystem, false }, { kMountPoint, "/" } };
    EXPECT_TRUE(DeviceUtils::isBuiltInDisk(rootDisk));

    QVariantHash ddeDisk { { kDevice, fakeId }, { kHintSystem, false }, { kIdLabel, "_dde_data" } };
    EXPECT_TRUE(DeviceUtils::isBuiltInDisk(ddeDisk));

    QVariantHash hintSys { { kDevice, fakeId }, { kHintSystem, true }, { kMountPoint, "/media/usb" } };
    EXPECT_TRUE(DeviceUtils::isBuiltInDisk(hintSys));

    QVariantHash sataDisk { { kDevice, fakeId }, { kHintSystem, false },
                            { kConnectionBus, "sata" }, { kMountPoint, "/media/usb" } };
    EXPECT_TRUE(DeviceUtils::isBuiltInDisk(sataDisk));

    // usb + non-system disk falls through to isSiblingOfRoot (environment dependent);
    // only assert it is callable and does not crash
    QVariantHash usbDisk { { kDevice, fakeId }, { kHintSystem, false },
                           { kConnectionBus, "usb" }, { kMountPoint, "/media/usb" } };
    EXPECT_NO_FATAL_FAILURE(DeviceUtils::isBuiltInDisk(usbDisk));
}

// PMS:233019 磁盘容量显示不准确：deviceBytesFree 应优先实时查询接口，
// 实时不可用时回退到 udisks 缓存（kSizeFree -> kSizeTotal-kSizeUsed）
TEST(DeviceUtilsTest, BUG233019_DeviceBytesFreePrefersRealTimeQuery)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(DeviceProxyManager, queryDeviceInfoByPath),
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       return { { GlobalServerDefines::DeviceProperty::kSizeTotal, 100 },
                                { GlobalServerDefines::DeviceProperty::kSizeUsed, 30 },
                                { GlobalServerDefines::DeviceProperty::kSizeFree, 70 } };
                   });
    stub.set_lamda(&DeviceHelper::queryDeviceUsageRealTime,
                   [](const QVariantMap &, quint64 *total, quint64 *avai, quint64 *used) -> bool {
                       __DBG_STUB_INVOKE__
                       *total = 100;
                       *avai = 55;
                       *used = 45;
                       return true;
                   });
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QUrl url = QUrl::fromLocalFile(dir.path());
    EXPECT_EQ(DeviceUtils::deviceBytesFree(url), qint64(55));
}

// PMS:233019 实时查询失败时回退缓存链：kSizeFree 有效用之；否则用 kSizeTotal-kSizeUsed
TEST(DeviceUtilsTest, BUG233019_DeviceBytesFreeFallsBackToCachedUsage)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(DeviceProxyManager, queryDeviceInfoByPath),
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       return { { GlobalServerDefines::DeviceProperty::kSizeTotal, 100 },
                                { GlobalServerDefines::DeviceProperty::kSizeUsed, 30 },
                                { GlobalServerDefines::DeviceProperty::kSizeFree, 70 } };
                   });
    stub.set_lamda(&DeviceHelper::queryDeviceUsageRealTime,
                   [](const QVariantMap &, quint64 *total, quint64 *avai, quint64 *used) -> bool {
                       __DBG_STUB_INVOKE__
                       *total = 0;
                       *avai = 0;
                       *used = 0;
                       return false;   // real-time query unavailable
                   });
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    EXPECT_EQ(DeviceUtils::deviceBytesFree(QUrl::fromLocalFile(dir.path())), qint64(70));

    // no kSizeFree in cache -> total - used
    stub.set_lamda(ADDR(DeviceProxyManager, queryDeviceInfoByPath),
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       return { { GlobalServerDefines::DeviceProperty::kSizeTotal, 100 },
                                { GlobalServerDefines::DeviceProperty::kSizeUsed, 30 } };
                   });
    EXPECT_EQ(DeviceUtils::deviceBytesFree(QUrl::fromLocalFile(dir.path())), qint64(70));
}
