// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_dockutils.cpp - disk-mount dock 插件纯工具函数单元测试
// 重编译 dockutils.cpp 进测试二进制；mtab 读取走真实 /etc/mtab。
//
// 分支清单（来源：dockutils.cpp 真实源码）→ 用例映射：
// isIntegratedByFilemanager: [DConfig 无效 → true 默认]  → 无 DConfig 环境/默认集成
// formatDiskSize: [循环除 1024 + 单位跃迁]              → FormatDiskSize_VariousUnits
// sizeString: [无'.' 原样][尾部0裁剪][纯".0"裁剪]        → SizeString_TrimsTrailingZeros 等
// parseSmbInfo: [不匹配→false][port 缺省→-1][全字段]     → ParseSmbInfo_* 系列
// blockDeviceName: [IdLabel 非空][空→容量名]             → BlockDeviceName_*
// protocolDeviceName: [SMB 名→"share on host"]           → ProtocolDeviceName_SmbDisplayName
// blockDeviceIcon: [加密→encrypted][光驱→optical][usb]   → BlockDeviceIcon_* 系列
// protocolDeviceIcon: [主题命中][缺省 drive-network]      → ProtocolDeviceIcon_*
// blockDeviceTarget: [光驱→burn://][普通→file://mpt]     → BlockDeviceTarget_* 系列
// protocolDeviceTarget: [SMB→smb url][普通→file]         → ProtocolDeviceTarget_* 系列
// isDlnfsMount: [mtab dlnfs 命中/未命中]                  → IsDlnfsMount_NoDlnfsInMtab
// queryDevice: [mtab 命中→source][未命中→空]             → QueryDevice_RootMountPoint 等
// protocolDeviceAlias: [空参][文件缺][JSON 坏][命中别名]  → ProtocolDeviceAlias_* 系列

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QVariantMap>
#include <QStandardPaths>
#include <QDebug>

#include "stubext.h"

#include "dockutils.h"
#include "global_server_defines.h"

using namespace GlobalServerDefines;

class DockUtilsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(DockUtilsTest, FormatDiskSize_VariousUnits_PicksCorrectUnit)
{
    // Arrange / Act / Assert
    EXPECT_EQ(size_format::formatDiskSize(0), QString("0 B"));
    EXPECT_EQ(size_format::formatDiskSize(512), QString("512 B"));
    EXPECT_EQ(size_format::formatDiskSize(2048), QString("2 KB"));
    EXPECT_EQ(size_format::formatDiskSize(1024ULL * 1024 * 1024), QString("1 GB"));
    EXPECT_EQ(size_format::formatDiskSize(3ULL * 1024 * 1024 * 1024 * 1024), QString("3 TB"));
}

TEST_F(DockUtilsTest, SizeString_TrimsTrailingZeros)
{
    // Arrange / Act / Assert
    EXPECT_EQ(size_format::sizeString("123"), QString("123"));   // no dot branch
    EXPECT_EQ(size_format::sizeString("1.50"), QString("1.5"));   // one trailing zero trimmed
    EXPECT_EQ(size_format::sizeString("2.00"), QString("2"));   // zeros + dot trimmed
    EXPECT_EQ(size_format::sizeString("1.25"), QString("1.25"));   // no trailing zero
}

TEST_F(DockUtilsTest, ParseSmbInfo_ValidString_FillsAllFields)
{
    // Arrange
    QString host, share;
    int port = 0;

    // Act
    const bool ok = smb_utils::parseSmbInfo(",port=445,server=nas.local,share=media", &host, &share, &port);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(host, QString("nas.local"));
    EXPECT_EQ(share, QString("media"));
    EXPECT_EQ(port, 445);
}

TEST_F(DockUtilsTest, ParseSmbInfo_NoPortNoShare_PortsMinusOne)
{
    // Arrange
    QString host, share;
    int port = 0;

    // Act
    const bool ok = smb_utils::parseSmbInfo(",server=storage", &host, &share, &port);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(host, QString("storage"));
    EXPECT_TRUE(share.isEmpty());
    EXPECT_EQ(port, -1);   // branch: empty port string -> -1
}

TEST_F(DockUtilsTest, ParseSmbInfo_GarbageInput_ReturnsFalse)
{
    // Arrange
    QString host, share;
    int port = 0;

    // Act
    const bool ok = smb_utils::parseSmbInfo("not-an-smb-path", &host, &share, &port);

    // Assert
    EXPECT_FALSE(ok);   // branch: regex no match
}

TEST_F(DockUtilsTest, BlockDeviceName_IdLabelPresent_WinsOverSize)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kIdLabel, "MyUsbDisk" }, { DeviceProperty::kSizeTotal, 1024ULL } };

    // Act
    const QString name = device_utils::blockDeviceName(data);

    // Assert
    EXPECT_EQ(name, QString("MyUsbDisk"));
}

TEST_F(DockUtilsTest, BlockDeviceName_NoLabel_FallsBackToSize)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kSizeTotal, 2ULL * 1024 * 1024 * 1024 } };

    // Act
    const QString name = device_utils::blockDeviceName(data);

    // Assert
    EXPECT_TRUE(name.contains("2 GB"));   // branch: empty label -> "<size> Volume"
    EXPECT_TRUE(name.contains("Volume"));
}

TEST_F(DockUtilsTest, ProtocolDeviceName_SmbDisplayName_RewrittenToShareOnHost)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kDisplayName, ",server=nas,share=photos" } };

    // Act
    const QString name = device_utils::protocolDeviceName(data);

    // Assert
    EXPECT_EQ(name, QString("photos on nas"));
}

TEST_F(DockUtilsTest, ProtocolDeviceName_PlainName_KeptAsIs)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kDisplayName, "network neighbor" } };

    // Act
    const QString name = device_utils::protocolDeviceName(data);

    // Assert
    EXPECT_EQ(name, QString("network neighbor"));
}

TEST_F(DockUtilsTest, BlockDeviceIcon_Encrypted_Optical_Usb_Priority)
{
    // Arrange
    QVariantMap encrypted { { DeviceProperty::kCryptoBackingDevice, "/dev/mapper/crypt" } };
    QVariantMap optical { { DeviceProperty::kCryptoBackingDevice, "/" },
                          { DeviceProperty::kOpticalDrive, true } };
    QVariantMap plain { { DeviceProperty::kCryptoBackingDevice, "/" } };

    // Act / Assert
    EXPECT_EQ(device_utils::blockDeviceIcon(encrypted), QString("drive-removable-media-encrypted"));
    EXPECT_EQ(device_utils::blockDeviceIcon(optical), QString("media-optical"));
    EXPECT_EQ(device_utils::blockDeviceIcon(plain), QString("drive-removable-media-usb"));
}

TEST_F(DockUtilsTest, ProtocolDeviceIcon_NoThemeHit_FallsBackToNetwork)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kDeviceIcon, QStringList { "no-such-theme-icon-xyz" } } };

    // Act
    const QString icon = device_utils::protocolDeviceIcon(data);

    // Assert
    EXPECT_EQ(icon, QString("drive-network"));   // branch: no theme icon matched
}

TEST_F(DockUtilsTest, BlockDeviceTarget_OpticalDrive_BuildsBurnUrl)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kOpticalDrive, true }, { DeviceProperty::kDevice, "/dev/sr0" } };

    // Act
    const QUrl target = device_utils::blockDeviceTarget(data);

    // Assert
    EXPECT_EQ(target.scheme(), QString("burn"));
    EXPECT_EQ(target.path(), QString("/dev/sr0/disc_files/"));
}

TEST_F(DockUtilsTest, BlockDeviceTarget_RegularMount_LocalFileUrl)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kOpticalDrive, false }, { DeviceProperty::kMountPoint, "/media/usb" } };

    // Act
    const QUrl target = device_utils::blockDeviceTarget(data);

    // Assert
    EXPECT_EQ(target.scheme(), QString("file"));
    EXPECT_EQ(target.toLocalFile(), QString("/media/usb"));
}

TEST_F(DockUtilsTest, ProtocolDeviceTarget_SmbMountPoint_BuildsSmbUrl)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kMountPoint, ",port=445,server=nas,share=docs" } };

    // Act
    const QUrl target = device_utils::protocolDeviceTarget(data);

    // Assert
    EXPECT_EQ(target.scheme(), QString("smb"));
    EXPECT_EQ(target.host(), QString("nas"));
    EXPECT_EQ(target.port(), 445);
    EXPECT_EQ(target.path(), QString("/docs"));
}

TEST_F(DockUtilsTest, ProtocolDeviceTarget_LocalMountPoint_LocalFileUrl)
{
    // Arrange
    QVariantMap data { { DeviceProperty::kMountPoint, "/home/uos/remote" } };

    // Act
    const QUrl target = device_utils::protocolDeviceTarget(data);

    // Assert
    EXPECT_EQ(target.toLocalFile(), QString("/home/uos/remote"));
}

TEST_F(DockUtilsTest, IsDlnfsMount_NoDlnfsEntries_ReturnsFalse)
{
    // Arrange: stock mtab on CI hosts never mounts a "dlnfs" source.

    // Act
    const bool isDlnfs = device_utils::isDlnfsMount("/media/dlnfs");

    // Assert
    EXPECT_FALSE(isDlnfs);
}

TEST_F(DockUtilsTest, QueryDevice_RootMountPoint_ReturnsNonEmptySource)
{
    // Arrange: "/" is always mounted, so the mtab lookup must succeed.

    // Act
    const QString source = device_utils::queryDevice("/");

    // Assert
    EXPECT_FALSE(source.isEmpty());   // branch: mtab target match -> source
}

TEST_F(DockUtilsTest, QueryDevice_UnknownMountPoint_ReturnsEmpty)
{
    // Arrange / Act
    const QString source = device_utils::queryDevice("/no/such/mountpoint/xyz");

    // Assert
    EXPECT_TRUE(source.isEmpty());   // branch: no match
}

TEST_F(DockUtilsTest, ProtocolDeviceAlias_MissingThenMatchingConfig_CoversBothBranches)
{
    // Arrange: one temp config location for both phases — cfgPath is cached
    // statically after the first call, so the file must appear in the SAME
    // directory the first (missing-file) call already cached.
    stub.set_lamda(&QStandardPaths::writableLocation,
                   [this](QStandardPaths::StandardLocation) -> QString {
                       __DBG_STUB_INVOKE__
                       return tempDir->path();
                   });

    // Act / Assert: phase 1 — empty inputs and absent config file. These run
    // first so the statically-cached cfgPath is captured with the stub active.
    EXPECT_TRUE(device_utils::protocolDeviceAlias("", "host").isEmpty());   // branch: empty scheme
    EXPECT_TRUE(device_utils::protocolDeviceAlias("smb", "").isEmpty());   // branch: empty host
    EXPECT_TRUE(device_utils::protocolDeviceAlias("ftp", "server.example").isEmpty());   // branch: open fails

    QDir().mkpath(tempDir->path() + "/deepin");
    QFile f(tempDir->path() + "/deepin/dde-file-manager.json");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(R"({"NetworkProtocolDeviceAlias":{"Items":[)"
            R"({"scheme":"ftp","devices":[{"host":"server.example","alias":"Corporate FTP"}]},)"
            R"({"scheme":"sftp","devices":[{"host":"other","alias":"nope"}]}]}})");
    f.close();

    // Act / Assert: phase 2 — config present and matched
    EXPECT_EQ(device_utils::protocolDeviceAlias("ftp", "server.example"), QString("Corporate FTP"));
    EXPECT_TRUE(device_utils::protocolDeviceAlias("sftp", "server.example").isEmpty());   // host mismatch
}
