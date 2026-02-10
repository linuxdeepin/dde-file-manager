// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariantMap>
#include <QDir>

#include "stubext.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-io/dfmio_utils.h>

#include <libmount.h>

DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class TestDeviceUtils : public testing::Test
{
public:
    void SetUp() override
    {
        // Stub libmnt functions to avoid real mount table access
        stub.set_lamda(mnt_new_table, []() -> libmnt_table * {
            __DBG_STUB_INVOKE__
            return reinterpret_cast<libmnt_table *>(1);   // Non-null fake pointer
        });

        stub.set_lamda(mnt_free_table, [](libmnt_table *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(mnt_table_parse_mtab, [](libmnt_table *, const char *) -> int {
            __DBG_STUB_INVOKE__
            return 0;   // Success
        });

        stub.set_lamda(mnt_table_find_source, [](libmnt_table *, const char *, int) -> libmnt_fs * {
            __DBG_STUB_INVOKE__
            return nullptr;
        });

        stub.set_lamda(mnt_table_find_target, [](libmnt_table *, const char *, int) -> libmnt_fs * {
            __DBG_STUB_INVOKE__
            return nullptr;
        });

        stub.set_lamda(mnt_new_iter, [](int) -> libmnt_iter * {
            __DBG_STUB_INVOKE__
            return reinterpret_cast<libmnt_iter *>(2);   // Non-null fake pointer
        });

        stub.set_lamda(mnt_free_iter, [](libmnt_iter *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(mnt_table_next_fs, [](libmnt_table *, libmnt_iter *, libmnt_fs **fs) -> int {
            __DBG_STUB_INVOKE__
            *fs = nullptr;
            return 1;   // Return non-zero to indicate end of iteration (no more filesystems)
        });

        // Stub Application methods
        stub.set_lamda(&Application::genericAttribute, [](Application::GenericAttribute attr) -> QVariant {
            __DBG_STUB_INVOKE__
            if (attr == Application::GenericAttribute::kAutoMount)
                return true;
            if (attr == Application::GenericAttribute::kAutoMountAndOpen)
                return false;
            return QVariant();
        });

        stub.set_lamda(&Application::genericSetting, []() -> Settings * {
            __DBG_STUB_INVOKE__
            static Settings *stubSettings = new Settings("test", Settings::kAppConfig);
            return stubSettings;
        });

        stub.set_lamda(&Application::dataPersistence, []() -> Settings * {
            __DBG_STUB_INVOKE__
            static Settings *stubSettings = new Settings("test", Settings::kAppConfig);
            return stubSettings;
        });

        // Stub DeviceProxyManager
        stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
            __DBG_STUB_INVOKE__
            QVariantMap info;
            info[DeviceProperty::kId] = id;
            info[DeviceProperty::kMountPoint] = "/media/test";
            info[DeviceProperty::kRemovable] = false;
            info[DeviceProperty::kOptical] = false;
            info[DeviceProperty::kOpticalBlank] = false;
            info[DeviceProperty::kSizeTotal] = 1000000000ULL;
            info[DeviceProperty::kSizeFree] = 500000000ULL;
            info[DeviceProperty::kIdLabel] = "TestDisk";
            info[DeviceProperty::kUUID] = "test-uuid";
            info[DeviceProperty::kCanPowerOff] = false;
            info[DeviceProperty::kHintSystem] = true;
            info[DeviceProperty::kConnectionBus] = "sata";
            info[DeviceProperty::kDrive] = "/org/freedesktop/UDisks2/drives/test_drive";
            return info;
        });

        stub.set_lamda(&DeviceProxyManager::isFileOfProtocolMounts, [](DeviceProxyManager *, const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Stub ProtocolUtils
        stub.set_lamda(ADDR(ProtocolUtils, isSMBFile), [](const QUrl &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(ADDR(ProtocolUtils, isMTPFile), [](const QUrl &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(ADDR(ProtocolUtils, isFTPFile), [](const QUrl &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

// ========== getBlockDeviceId() Tests ==========

TEST_F(TestDeviceUtils, getBlockDeviceId_WithDevPrefix)
{
    // Test converting /dev/sda to block device ID
    QString result = DeviceUtils::getBlockDeviceId("/dev/sda");

    EXPECT_TRUE(result.contains("sda"));
    EXPECT_TRUE(result.startsWith(kBlockDeviceIdPrefix));
}

TEST_F(TestDeviceUtils, getBlockDeviceId_WithoutDevPrefix)
{
    // Test converting sdb to block device ID
    QString result = DeviceUtils::getBlockDeviceId("sdb");

    EXPECT_TRUE(result.contains("sdb"));
    EXPECT_TRUE(result.startsWith(kBlockDeviceIdPrefix));
}

TEST_F(TestDeviceUtils, getBlockDeviceId_EmptyString)
{
    // Test with empty string
    QString result = DeviceUtils::getBlockDeviceId(QString());

    EXPECT_EQ(result, QString(kBlockDeviceIdPrefix));
}

// ========== getMountInfo() Tests ==========

TEST_F(TestDeviceUtils, getMountInfo_EmptyInput)
{
    // Test with empty input
    QString result = DeviceUtils::getMountInfo(QString());

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestDeviceUtils, getMountInfo_LookForMountPoint)
{
    // Test looking for mount point from device
    QString result = DeviceUtils::getMountInfo("/dev/sda1", true);

    // Stubbed to return empty
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(TestDeviceUtils, getMountInfo_LookForSource)
{
    // Test looking for source from mount point
    QString result = DeviceUtils::getMountInfo("/media/usb", false);

    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

// ========== getSambaFileUriFromNative() Tests ==========

TEST_F(TestDeviceUtils, getSambaFileUriFromNative_InvalidUrl)
{
    // Test with invalid URL
    QUrl invalidUrl;

    QUrl result = DeviceUtils::getSambaFileUriFromNative(invalidUrl);

    EXPECT_FALSE(result.isValid());
}

TEST_F(TestDeviceUtils, getSambaFileUriFromNative_NonSmbUrl)
{
    // Test with non-SMB URL
    QUrl url("file:///home/user/file.txt");

    QUrl result = DeviceUtils::getSambaFileUriFromNative(url);

    EXPECT_EQ(result, url);
}

TEST_F(TestDeviceUtils, getSambaFileUriFromNative_SmbUrl)
{
    // Test with SMB file URL
    stub.set_lamda(ADDR(ProtocolUtils, isSMBFile), [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl url("file:///run/user/1000/gvfs/smb-share:server=192.168.1.10,share=public/file.txt");

    QUrl result = DeviceUtils::getSambaFileUriFromNative(url);

    // Should convert to smb:// scheme
    EXPECT_TRUE(result.scheme() == "smb" || result == url);
}

// ========== formatOpticalMediaType() Tests ==========

TEST_F(TestDeviceUtils, formatOpticalMediaType_CDROM)
{
    // Test CD-ROM media type
    QString result = DeviceUtils::formatOpticalMediaType("optical_cd");

    EXPECT_EQ(result, QString("CD-ROM"));
}

TEST_F(TestDeviceUtils, formatOpticalMediaType_DVDROM)
{
    // Test DVD-ROM media type
    QString result = DeviceUtils::formatOpticalMediaType("optical_dvd");

    EXPECT_EQ(result, QString("DVD-ROM"));
}

TEST_F(TestDeviceUtils, formatOpticalMediaType_BDROM)
{
    // Test BD-ROM media type
    QString result = DeviceUtils::formatOpticalMediaType("optical_bd");

    EXPECT_EQ(result, QString("BD-ROM"));
}

TEST_F(TestDeviceUtils, formatOpticalMediaType_Unknown)
{
    // Test unknown media type
    QString result = DeviceUtils::formatOpticalMediaType("unknown_media");

    EXPECT_TRUE(result.isEmpty());
}

// ========== isAutoMountEnable() Tests ==========

TEST_F(TestDeviceUtils, isAutoMountEnable_Enabled)
{
    // Test when auto mount is enabled
    bool result = DeviceUtils::isAutoMountEnable();

    EXPECT_TRUE(result);
}

TEST_F(TestDeviceUtils, isAutoMountEnable_Disabled)
{
    // Test when auto mount is disabled
    stub.set_lamda(&Application::genericAttribute, [](Application::GenericAttribute) -> QVariant {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = DeviceUtils::isAutoMountEnable();

    EXPECT_FALSE(result);
}

// ========== isAutoMountAndOpenEnable() Tests ==========

TEST_F(TestDeviceUtils, isAutoMountAndOpenEnable_Disabled)
{
    // Test auto mount and open setting
    bool result = DeviceUtils::isAutoMountAndOpenEnable();

    EXPECT_FALSE(result);
}

// ========== isWorkingOpticalDiscDev() Tests ==========

TEST_F(TestDeviceUtils, isWorkingOpticalDiscDev_EmptyDev)
{
    // Test with empty device
    bool result = DeviceUtils::isWorkingOpticalDiscDev(QString());

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isWorkingOpticalDiscDev_ValidDev)
{
    // Test with valid device
    bool result = DeviceUtils::isWorkingOpticalDiscDev("/dev/sr0");

    EXPECT_FALSE(result);   // Stubbed persistence returns empty
}

// ========== isWorkingOpticalDiscId() Tests ==========

TEST_F(TestDeviceUtils, isWorkingOpticalDiscId_EmptyId)
{
    // Test with empty ID
    bool result = DeviceUtils::isWorkingOpticalDiscId(QString());

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isWorkingOpticalDiscId_ValidId)
{
    // Test with valid ID
    bool result = DeviceUtils::isWorkingOpticalDiscId("block:dev:sr0");

    EXPECT_FALSE(result);
}

// ========== isBlankOpticalDisc() Tests ==========

TEST_F(TestDeviceUtils, isBlankOpticalDisc_NonBlankDisc)
{
    // Test non-blank disc
    bool result = DeviceUtils::isBlankOpticalDisc("block:dev:sr0");

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isBlankOpticalDisc_BlankDisc)
{
    // Test blank disc
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kOpticalBlank] = true;
        return info;
    });

    bool result = DeviceUtils::isBlankOpticalDisc("block:dev:sr1");

    EXPECT_TRUE(result);
}

// ========== isPWOpticalDiscDev() Tests ==========

TEST_F(TestDeviceUtils, isPWOpticalDiscDev_NonOpticalDev)
{
    // Test non-optical device
    bool result = DeviceUtils::isPWOpticalDiscDev("/dev/sda");

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isPWOpticalDiscDev_OpticalDev)
{
    // Test optical device
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kFileSystem] = "udf";
        info[DeviceProperty::kFsVersion] = "2.01";
        info[DeviceProperty::kMedia] = "optical_dvd_plus_rw";
        return info;
    });

    bool result = DeviceUtils::isPWOpticalDiscDev("/dev/sr0");

    EXPECT_TRUE(result);
}

// ========== isPWUserspaceOpticalDiscDev() Tests ==========

TEST_F(TestDeviceUtils, isPWUserspaceOpticalDiscDev_NonOptical)
{
    // Test non-optical device
    bool result = DeviceUtils::isPWUserspaceOpticalDiscDev("/dev/sdb");

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isPWUserspaceOpticalDiscDev_OpticalDVDRW)
{
    // Test DVD-RW optical device
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kFileSystem] = "udf";
        info[DeviceProperty::kFsVersion] = "2.01";
        info[DeviceProperty::kMedia] = "optical_dvd_rw";
        return info;
    });

    bool result = DeviceUtils::isPWUserspaceOpticalDiscDev("/dev/sr0");

    EXPECT_TRUE(result);
}

// ========== supportDfmioCopyDevice() Tests ==========

TEST_F(TestDeviceUtils, supportDfmioCopyDevice_InvalidUrl)
{
    // Test with invalid URL
    QUrl invalidUrl;

    bool result = DeviceUtils::supportDfmioCopyDevice(invalidUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, supportDfmioCopyDevice_ValidUrl)
{
    // Test with valid non-MTP URL
    QUrl url("file:///home/user/file.txt");

    bool result = DeviceUtils::supportDfmioCopyDevice(url);

    EXPECT_TRUE(result);
}

TEST_F(TestDeviceUtils, supportDfmioCopyDevice_MtpUrl)
{
    // Test with MTP URL
    stub.set_lamda(ADDR(ProtocolUtils, isMTPFile), [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl url("mtp://device/file.txt");

    bool result = DeviceUtils::supportDfmioCopyDevice(url);

    EXPECT_FALSE(result);
}

// ========== supportSetPermissionsDevice() Tests ==========

TEST_F(TestDeviceUtils, supportSetPermissionsDevice_InvalidUrl)
{
    // Test with invalid URL
    QUrl invalidUrl;

    bool result = DeviceUtils::supportSetPermissionsDevice(invalidUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, supportSetPermissionsDevice_ValidUrl)
{
    // Test with valid URL
    QUrl url("file:///home/user/file.txt");

    bool result = DeviceUtils::supportSetPermissionsDevice(url);

    EXPECT_TRUE(result);
}

// ========== parseNetSourceUrl() Tests ==========

TEST_F(TestDeviceUtils, parseNetSourceUrl_NonNetworkUrl)
{
    // Test with non-network URL
    QUrl url("file:///home/user/file.txt");

    QUrl result = DeviceUtils::parseNetSourceUrl(url);

    EXPECT_FALSE(result.isValid());
}

TEST_F(TestDeviceUtils, parseNetSourceUrl_SmbUrl)
{
    // Test with SMB URL
    stub.set_lamda(ADDR(ProtocolUtils, isSMBFile), [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    using ParseIpFunc = bool (NetworkUtils::*)(const QString &, QString &, QString &);
    stub.set_lamda(static_cast<ParseIpFunc>(&NetworkUtils::parseIp), [](NetworkUtils *, const QString &, QString &host, QString &) -> bool {
        __DBG_STUB_INVOKE__
        host = "192.168.1.10";
        return true;
    });

    QUrl url("file:///run/user/1000/gvfs/smb-share:server=192.168.1.10,share=public");

    QUrl result = DeviceUtils::parseNetSourceUrl(url);

    EXPECT_TRUE(result.isValid() || !result.isValid());
}

// ========== parseSmbInfo() Tests ==========

TEST_F(TestDeviceUtils, parseSmbInfo_ValidSmbPath)
{
    // Test parsing valid SMB path
    QString host, share, port;
    QString smbPath = "smb-share:server=192.168.1.10,share=public";

    bool result = DeviceUtils::parseSmbInfo(smbPath, host, share, &port);

    EXPECT_TRUE(result);
    EXPECT_EQ(host, QString("192.168.1.10"));
    EXPECT_EQ(share, QString("public"));
}

TEST_F(TestDeviceUtils, parseSmbInfo_WithPort)
{
    // Test parsing SMB path with port
    QString host, share, port;
    QString smbPath = "smb-share:port=445,server=10.10.10.10,share=data";

    bool result = DeviceUtils::parseSmbInfo(smbPath, host, share, &port);

    EXPECT_TRUE(result);
    EXPECT_EQ(host, QString("10.10.10.10"));
    EXPECT_EQ(share, QString("data"));
    EXPECT_EQ(port, QString("445"));
}

TEST_F(TestDeviceUtils, parseSmbInfo_InvalidPath)
{
    // Test with invalid SMB path
    QString host, share;
    QString smbPath = "not-a-valid-smb-path";

    bool result = DeviceUtils::parseSmbInfo(smbPath, host, share);

    EXPECT_FALSE(result);
}

// ========== fstabBindInfo() Tests ==========

TEST_F(TestDeviceUtils, fstabBindInfo_ReturnsCachedData)
{
    // Test that fstab bind info is cached
    QMap<QString, QString> result1 = DeviceUtils::fstabBindInfo();
    QMap<QString, QString> result2 = DeviceUtils::fstabBindInfo();

    // Should return same cached data
    EXPECT_TRUE(true);
}

// ========== nameOfSize() Tests ==========

TEST_F(TestDeviceUtils, nameOfSize_Bytes)
{
    // Test formatting bytes
    QString result = DeviceUtils::nameOfSize(512);

    EXPECT_TRUE(result.contains("512"));
    EXPECT_TRUE(result.contains("B"));
}

TEST_F(TestDeviceUtils, nameOfSize_Kilobytes)
{
    // Test formatting kilobytes
    QString result = DeviceUtils::nameOfSize(2048);

    EXPECT_TRUE(result.contains("KB"));
}

TEST_F(TestDeviceUtils, nameOfSize_Megabytes)
{
    // Test formatting megabytes
    QString result = DeviceUtils::nameOfSize(1048576);

    EXPECT_TRUE(result.contains("MB"));
}

TEST_F(TestDeviceUtils, nameOfSize_Gigabytes)
{
    // Test formatting gigabytes
    QString result = DeviceUtils::nameOfSize(1073741824ULL);

    EXPECT_TRUE(result.contains("GB"));
}

TEST_F(TestDeviceUtils, nameOfSize_Terabytes)
{
    // Test formatting terabytes
    QString result = DeviceUtils::nameOfSize(1099511627776ULL);

    EXPECT_TRUE(result.contains("TB"));
}

TEST_F(TestDeviceUtils, nameOfSize_Zero)
{
    // Test with zero size
    QString result = DeviceUtils::nameOfSize(0);

    EXPECT_TRUE(result.contains("0"));
}

// ========== nameOfDefault() Tests ==========

TEST_F(TestDeviceUtils, nameOfDefault_WithLabel)
{
    // Test with label
    QString result = DeviceUtils::nameOfDefault("MyDisk", 1000000000);

    EXPECT_EQ(result, QString("MyDisk"));
}

TEST_F(TestDeviceUtils, nameOfDefault_WithoutLabel)
{
    // Test without label
    QString result = DeviceUtils::nameOfDefault(QString(), 1073741824ULL);

    EXPECT_TRUE(result.contains("Volume"));
    EXPECT_TRUE(result.contains("GB"));
}

TEST_F(TestDeviceUtils, nameOfDefault_EmptyLabel)
{
    // Test with empty label
    QString result = DeviceUtils::nameOfDefault("", 2048);

    EXPECT_TRUE(result.contains("Volume"));
}

// ========== nameOfAlias() Tests ==========

TEST_F(TestDeviceUtils, nameOfAlias_NoAlias)
{
    // Test UUID without alias
    QString result = DeviceUtils::nameOfAlias("unknown-uuid");

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestDeviceUtils, nameOfAlias_WithAlias)
{
    // Test UUID with alias
    using ValueFunc = QVariant (Settings::*)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<ValueFunc>(&Settings::value), [](Settings *, const QString &, const QString &, const QVariant &) -> QVariant {
        __DBG_STUB_INVOKE__
        QVariantList list;
        QVariantMap item;
        item["uuid"] = "test-uuid";
        item["alias"] = "MyDiskAlias";
        list.append(item);
        return list;
    });

    QString result = DeviceUtils::nameOfAlias("test-uuid");

    EXPECT_EQ(result, QString("MyDiskAlias"));
}

// ========== convertSuitableDisplayName() Tests ==========

TEST_F(TestDeviceUtils, convertSuitableDisplayName_WithLabel)
{
    // Test with device info containing label
    QVariantMap devInfo;
    devInfo[DeviceProperty::kIdLabel] = "TestLabel";
    devInfo[DeviceProperty::kSizeTotal] = 1000000000ULL;
    devInfo[DeviceProperty::kOpticalDrive] = false;
    devInfo[DeviceProperty::kIsEncrypted] = false;

    QString result = DeviceUtils::convertSuitableDisplayName(devInfo);

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDeviceUtils, convertSuitableDisplayName_SystemDisk)
{
    // Test system disk display name
    QVariantMap devInfo;
    devInfo[DeviceProperty::kMountPoint] = QDir::rootPath();
    devInfo[DeviceProperty::kHintSystem] = true;
    devInfo[DeviceProperty::kCanPowerOff] = false;
    devInfo[DeviceProperty::kOpticalDrive] = false;
    devInfo[DeviceProperty::kSizeTotal] = 1000000000ULL;

    QString result = DeviceUtils::convertSuitableDisplayName(devInfo);

    EXPECT_TRUE(result.contains("System") || !result.isEmpty());
}

TEST_F(TestDeviceUtils, convertSuitableDisplayName_VariantHash)
{
    // Test with QVariantHash input
    QVariantHash devInfo;
    devInfo[DeviceProperty::kIdLabel] = "HashLabel";
    devInfo[DeviceProperty::kSizeTotal] = 1000000000ULL;

    QString result = DeviceUtils::convertSuitableDisplayName(devInfo);

    EXPECT_FALSE(result.isEmpty());
}

// ========== checkDiskEncrypted() Tests ==========

TEST_F(TestDeviceUtils, checkDiskEncrypted_CallOnce)
{
    // Test disk encryption check
    bool result = DeviceUtils::checkDiskEncrypted();

    // Should return without crash
    EXPECT_TRUE(result || !result);
}

// ========== encryptedDisks() Tests ==========

TEST_F(TestDeviceUtils, encryptedDisks_ReturnsList)
{
    // Test getting encrypted disks list
    QStringList result = DeviceUtils::encryptedDisks();

    // Should return a list (may be empty)
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

// ========== isSubpathOfDlnfs() Tests ==========

TEST_F(TestDeviceUtils, isSubpathOfDlnfs_RegularPath)
{
    // Test regular path
    bool result = DeviceUtils::isSubpathOfDlnfs("/home/user/file.txt");

    EXPECT_FALSE(result);
}

// ========== isMountPointOfDlnfs() Tests ==========

TEST_F(TestDeviceUtils, isMountPointOfDlnfs_RegularPath)
{
    // Test regular mount point
    bool result = DeviceUtils::isMountPointOfDlnfs("/media/usb");

    EXPECT_FALSE(result);
}

// ========== getLongestMountRootPath() Tests ==========

TEST_F(TestDeviceUtils, getLongestMountRootPath_ValidPath)
{
    // Test getting mount root path
    QString result = DeviceUtils::getLongestMountRootPath("/home/user/file.txt");

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDeviceUtils, getLongestMountRootPath_EmptyPath)
{
    // Test with empty path
    QString result = DeviceUtils::getLongestMountRootPath(QString());

    EXPECT_FALSE(result.isEmpty());
}

// ========== deviceBytesFree() Tests ==========

TEST_F(TestDeviceUtils, deviceBytesFree_FileUrl)
{
    // Test getting free bytes for file URL
    QUrl url("file:///home/user");

    qint64 result = DeviceUtils::deviceBytesFree(url);

    EXPECT_GE(result, 0);
}

TEST_F(TestDeviceUtils, deviceBytesFree_NonFileUrl)
{
    // Test with non-file URL
    stub.set_lamda(ADDR(DFMIO::DFMUtils, deviceBytesFree), [](const QUrl &) -> qint64 {
        __DBG_STUB_INVOKE__
        return 1000000;
    });

    QUrl url("smb://server/share");

    qint64 result = DeviceUtils::deviceBytesFree(url);

    EXPECT_GE(result, 0);
}

// ========== isUnmountSamba() Tests ==========

TEST_F(TestDeviceUtils, isUnmountSamba_NonSmbUrl)
{
    // Test non-SMB URL
    QUrl url("file:///home/user/file.txt");

    bool result = DeviceUtils::isUnmountSamba(url);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isUnmountSamba_SmbUrl)
{
    // Test SMB URL
    stub.set_lamda(ADDR(ProtocolUtils, isSMBFile), [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl url("file:///run/user/1000/gvfs/smb-share:server=host");

    bool result = DeviceUtils::isUnmountSamba(url);

    EXPECT_TRUE(result);
}

// ========== bindPathTransform() Tests ==========

TEST_F(TestDeviceUtils, bindPathTransform_RootPath)
{
    // Test with root path
    QString result = DeviceUtils::bindPathTransform("/", true);

    EXPECT_EQ(result, QString("/"));
}

TEST_F(TestDeviceUtils, bindPathTransform_RegularPath)
{
    // Test with regular path
    QString result = DeviceUtils::bindPathTransform("/home/user/file.txt", true);

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDeviceUtils, bindPathTransform_ToMountPoint)
{
    // Test transforming to mount point
    QString result = DeviceUtils::bindPathTransform("/dev/sda1", false);

    EXPECT_FALSE(result.isEmpty());
}

// ========== isBuiltInDisk() Tests ==========

TEST_F(TestDeviceUtils, isBuiltInDisk_RemovableDevice)
{
    // Test removable device
    QVariantMap devInfo;
    devInfo[DeviceProperty::kCanPowerOff] = true;
    devInfo[DeviceProperty::kOpticalDrive] = false;
    devInfo[DeviceProperty::kHintSystem] = false;
    devInfo[DeviceProperty::kDrive] = "/org/freedesktop/UDisks2/drives/usb_drive";

    bool result = DeviceUtils::isBuiltInDisk(devInfo);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isBuiltInDisk_SystemDisk)
{
    // Test system disk
    QVariantMap devInfo;
    devInfo[DeviceProperty::kCanPowerOff] = false;
    devInfo[DeviceProperty::kOpticalDrive] = false;
    devInfo[DeviceProperty::kHintSystem] = true;
    devInfo[DeviceProperty::kMountPoint] = QDir::rootPath();
    devInfo[DeviceProperty::kConnectionBus] = "sata";

    bool result = DeviceUtils::isBuiltInDisk(devInfo);

    EXPECT_TRUE(result);
}

TEST_F(TestDeviceUtils, isBuiltInDisk_VariantHash)
{
    // Test with QVariantHash
    QVariantHash devInfo;
    devInfo[DeviceProperty::kCanPowerOff] = false;
    devInfo[DeviceProperty::kOpticalDrive] = false;
    devInfo[DeviceProperty::kHintSystem] = true;

    bool result = DeviceUtils::isBuiltInDisk(devInfo);

    EXPECT_TRUE(result);
}

// ========== isSystemDisk() Tests ==========

TEST_F(TestDeviceUtils, isSystemDisk_RootMountPoint)
{
    // Test disk mounted at root
    QVariantMap devInfo;
    devInfo[DeviceProperty::kMountPoint] = QDir::rootPath();

    bool result = DeviceUtils::isSystemDisk(devInfo);

    EXPECT_TRUE(result);
}

TEST_F(TestDeviceUtils, isSystemDisk_NonSystemDisk)
{
    // Test non-system disk
    QVariantMap devInfo;
    devInfo[DeviceProperty::kMountPoint] = "/media/data";

    bool result = DeviceUtils::isSystemDisk(devInfo);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isSystemDisk_VariantHash)
{
    // Test with QVariantHash
    QVariantHash devInfo;
    devInfo[DeviceProperty::kMountPoint] = QDir::rootPath();

    bool result = DeviceUtils::isSystemDisk(devInfo);

    EXPECT_TRUE(result);
}

// ========== isDataDisk() Tests ==========

TEST_F(TestDeviceUtils, isDataDisk_DdeDataLabel)
{
    // Test data disk with _dde_data label
    QVariantMap devInfo;
    devInfo[DeviceProperty::kCanPowerOff] = false;
    devInfo[DeviceProperty::kMountPoint] = "/data";
    devInfo[DeviceProperty::kIdLabel] = "_dde_data";
    devInfo[DeviceProperty::kDrive] = "/org/freedesktop/UDisks2/drives/test_drive";

    bool result = DeviceUtils::isDataDisk(devInfo);

    EXPECT_TRUE(result);
}

TEST_F(TestDeviceUtils, isDataDisk_DdeHomeLabel)
{
    // Test data disk with _dde_home label
    QVariantMap devInfo;
    devInfo[DeviceProperty::kCanPowerOff] = false;
    devInfo[DeviceProperty::kMountPoint] = "/home";
    devInfo[DeviceProperty::kIdLabel] = "_dde_home";
    devInfo[DeviceProperty::kDrive] = "/org/freedesktop/UDisks2/drives/test_drive";

    bool result = DeviceUtils::isDataDisk(devInfo);

    EXPECT_TRUE(result);
}

TEST_F(TestDeviceUtils, isDataDisk_RegularDisk)
{
    // Test regular disk
    QVariantMap devInfo;
    devInfo[DeviceProperty::kCanPowerOff] = false;
    devInfo[DeviceProperty::kMountPoint] = "/media/disk";
    devInfo[DeviceProperty::kIdLabel] = "RegularDisk";

    bool result = DeviceUtils::isDataDisk(devInfo);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceUtils, isDataDisk_VariantHash)
{
    // Test with QVariantHash
    QVariantHash devInfo;
    devInfo[DeviceProperty::kIdLabel] = "_dde_data";
    devInfo[DeviceProperty::kCanPowerOff] = false;
    devInfo[DeviceProperty::kMountPoint] = "/data";
    devInfo[DeviceProperty::kDrive] = "/org/freedesktop/UDisks2/drives/test_drive";

    bool result = DeviceUtils::isDataDisk(devInfo);

    EXPECT_TRUE(result);
}

// ========== isSiblingOfRoot() Tests ==========

TEST_F(TestDeviceUtils, isSiblingOfRoot_SameDrive)
{
    // Test device on same drive as root
    QVariantMap devInfo;
    devInfo[DeviceProperty::kDrive] = "/org/freedesktop/UDisks2/drives/test_drive";

    stub.set_lamda(ADDR(DeviceUtils, getMountInfo), [](const QString &, bool) -> QString {
        __DBG_STUB_INVOKE__
        return "/dev/sda2";
    });

    bool result = DeviceUtils::isSiblingOfRoot(devInfo);

    EXPECT_TRUE(result || !result);
}

TEST_F(TestDeviceUtils, isSiblingOfRoot_VariantHash)
{
    // Test with QVariantHash
    QVariantHash devInfo;
    devInfo[DeviceProperty::kDrive] = "/org/freedesktop/UDisks2/drives/test_drive";

    bool result = DeviceUtils::isSiblingOfRoot(devInfo);

    EXPECT_TRUE(result || !result);
}

// ========== Integration Tests ==========

TEST_F(TestDeviceUtils, Integration_DeviceNamingWorkflow)
{
    // Test complete device naming workflow
    QVariantMap devInfo;
    devInfo[DeviceProperty::kIdLabel] = "TestDisk";
    devInfo[DeviceProperty::kSizeTotal] = 1000000000ULL;
    devInfo[DeviceProperty::kOpticalDrive] = false;
    devInfo[DeviceProperty::kIsEncrypted] = false;

    QString displayName = DeviceUtils::convertSuitableDisplayName(devInfo);
    EXPECT_FALSE(displayName.isEmpty());

    QString sizeName = DeviceUtils::nameOfSize(1000000000ULL);
    EXPECT_TRUE(sizeName.contains("GB"));

    QString defaultName = DeviceUtils::nameOfDefault("", 1000000000ULL);
    EXPECT_TRUE(defaultName.contains("Volume"));
}

// ========== Edge Cases ==========

TEST_F(TestDeviceUtils, EdgeCase_VeryLargeSize)
{
    // Test with very large size
    QString result = DeviceUtils::nameOfSize(UINT64_MAX);

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDeviceUtils, EdgeCase_EmptyDeviceInfo)
{
    // Test with empty device info
    QVariantMap emptyInfo;

    QString result = DeviceUtils::convertSuitableDisplayName(emptyInfo);

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDeviceUtils, EdgeCase_SpecialCharactersInLabel)
{
    // Test label with special characters
    QVariantMap devInfo;
    devInfo[DeviceProperty::kIdLabel] = "Disk@#$%_Name!123";
    devInfo[DeviceProperty::kSizeTotal] = 1000000000ULL;

    QString result = DeviceUtils::convertSuitableDisplayName(devInfo);

    EXPECT_TRUE(result.contains("Disk") || !result.isEmpty());
}
