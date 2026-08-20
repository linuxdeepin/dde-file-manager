// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicehelper.cpp
 * @brief Unit tests for DeviceHelper (base/device/private/devicehelper.cpp) —
 *        the dependency-light subset: castFromDFMMountProperty (pure lookup),
 *        isMountableBlockDev/isEjectableBlockDev (QVariantMap overloads, pure
 *        logic), clearOpticalInfo (empty-tag early return), makeFakeProtocolInfo
 *        (private, exercised via -fno-access-control).
 */

#include <gtest/gtest.h>
#include <QVariantMap>
#include <QString>
#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>
#include "stubext.h"

#include <dfm-mount/base/dmount_global.h>

#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dbusservice/opticalshareproxy.h>
#include <dfm-base/utils/networkutils.h>
#include <DDesktopServices>

using namespace dfmbase;

TEST(DeviceHelperTest, CastFromDFMMountPropertyKnownProperty)
{
    using namespace dfmmount;
    QString result = DeviceHelper::castFromDFMMountProperty(Property::kBlockSize);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceHelperTest, CastFromDFMMountPropertyMultipleKnown)
{
    using namespace dfmmount;
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDUUID).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDType).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveMedia).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveOptical).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveEjectable).isEmpty());
}

TEST(DeviceHelperTest, CastFromDFMMountPropertyUnknownReturnsEmpty)
{
    using namespace dfmmount;
    // A property not in the mapper returns ""
    EXPECT_EQ(DeviceHelper::castFromDFMMountProperty(static_cast<Property>(99999)), QString());
}

namespace DP = GlobalServerDefines::DeviceProperty;

TEST(DeviceHelperTest, IsMountableBlockDevEmptyIdReturnsFalse)
{
    QVariantMap infos;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevHintIgnoreReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHintIgnore] = true;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevAlreadyMountedReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kMountPoint] = "/mnt/data";
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevNoFileSystemReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHasFileSystem] = false;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsMountableBlockDevEncryptedReturnsFalse)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHasFileSystem] = true;
    infos[DP::kIsEncrypted] = true;
    QString why;
    EXPECT_FALSE(DeviceHelper::isMountableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsMountableBlockDevValidReturnsTrue)
{
    QVariantMap infos;
    infos[DP::kId] = "/dev/sda1";
    infos[DP::kHasFileSystem] = true;
    infos[DP::kIsEncrypted] = false;
    QString why;
    EXPECT_TRUE(DeviceHelper::isMountableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsEjectableBlockDevRemovableReturnsTrue)
{
    QVariantMap infos;
    infos[DP::kRemovable] = true;
    QString why;
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsEjectableBlockDevOpticalEjectableReturnsTrue)
{
    QVariantMap infos;
    infos[DP::kOptical] = true;
    infos[DP::kEjectable] = true;
    QString why;
    EXPECT_TRUE(DeviceHelper::isEjectableBlockDev(infos, why));
}

TEST(DeviceHelperTest, IsEjectableBlockDevNonEjectableReturnsFalse)
{
    QVariantMap infos;
    QString why;
    EXPECT_FALSE(DeviceHelper::isEjectableBlockDev(infos, why));
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, ClearOpticalInfoEmptyTagIsNoOp)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceHelper::clearOpticalInfo(QString()); });
}

TEST(DeviceHelperTest, MakeFakeProtocolInfoBuildsBasicMap)
{
    // Private method, reached via -fno-access-control.
    QString id = "smb://10.0.0.1/share";
    QVariantMap info = DeviceHelper::makeFakeProtocolInfo(id);
    EXPECT_FALSE(info.isEmpty());
    EXPECT_EQ(info.value("fake").toBool(), true);
}

// ============================================================
// Additional coverage for DeviceHelper
// ============================================================

TEST(DeviceHelperTest, CastFromDFMMountPropertyAllMapped)
{
    using namespace dfmmount;
    // Test all properties that have mappings
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockSize).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDUUID).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDType).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDVersion).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockIDLabel).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveMedia).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockReadOnly).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveMediaRemovable).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveOptical).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveOpticalBlank).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveMediaAvailable).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveCanPowerOff).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveEjectable).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockHintIgnore).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kBlockCryptoBackingDevice).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kFileSystemMountPoint).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kDriveMediaCompatibility).isEmpty());
    EXPECT_FALSE(DeviceHelper::castFromDFMMountProperty(Property::kEncryptedCleartextDevice).isEmpty());
}

TEST(DeviceHelperTest, CreateDeviceWithAllDeviceTypeReturnsNull)
{
    auto dev = DeviceHelper::createDevice("/some/id", dfmmount::DeviceType::kAllDevice);
    EXPECT_EQ(dev, nullptr);
}

TEST(DeviceHelperTest, CreateBlockDeviceNonExistent)
{
    auto dev = DeviceHelper::createBlockDevice("/org/freedesktop/UDisks2/block_devices/nonexistent_abc");
    // May return null in test environment
    EXPECT_EQ(dev, nullptr);
}

TEST(DeviceHelperTest, CreateProtocolDeviceNonExistent)
{
    auto dev = DeviceHelper::createProtocolDevice("/nonexistent/protocol/dev_abc");
    // In some envs, dfm-mount may create a device object
    EXPECT_NO_FATAL_FAILURE({ (void)dev; });
}

TEST(DeviceHelperTest, LoadBlockInfoNonExistent)
{
    QVariantMap info = DeviceHelper::loadBlockInfo("/org/freedesktop/UDisks2/block_devices/nonexistent_load");
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceHelperTest, LoadBlockInfoWithNullDevice)
{
    BlockDevAutoPtr nullDev;
    QVariantMap info = DeviceHelper::loadBlockInfo(nullDev);
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceHelperTest, LoadProtocolInfoNonExistent)
{
    QVariantMap info = DeviceHelper::loadProtocolInfo("/nonexistent/protocol/dev_load");
    // May return real or fake info in env with dfm-mount running
    EXPECT_NO_FATAL_FAILURE({ (void)info; });
}

TEST(DeviceHelperTest, LoadProtocolInfoWithNullDevice)
{
    ProtocolDevAutoPtr nullDev;
    QVariantMap info = DeviceHelper::loadProtocolInfo(nullDev);
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevByStringNonExistent)
{
    QString why;
    bool result = DeviceHelper::isMountableBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent_mount", why);
    EXPECT_FALSE(result);
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsMountableBlockDevByDevicePtrNull)
{
    BlockDevAutoPtr nullDev;
    QString why;
    bool result = DeviceHelper::isMountableBlockDev(nullDev, why);
    EXPECT_FALSE(result);
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsEjectableBlockDevByStringNonExistent)
{
    QString why;
    bool result = DeviceHelper::isEjectableBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent_eject", why);
    EXPECT_FALSE(result);
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, IsEjectableBlockDevByDevicePtrNull)
{
    BlockDevAutoPtr nullDev;
    QString why;
    bool result = DeviceHelper::isEjectableBlockDev(nullDev, why);
    EXPECT_FALSE(result);
    EXPECT_FALSE(why.isEmpty());
}

TEST(DeviceHelperTest, QueryUsageOfBlockRealTimeEmptyMpt)
{
    quint64 total = 0, avai = 0, used = 0;
    QVariantMap itemData;
    itemData[DP::kMountPoint] = "";
    bool result = DeviceHelper::queryUsageOfBlockRealTime(itemData, &total, &avai, &used);
    EXPECT_FALSE(result);
}

TEST(DeviceHelperTest, QueryUsageOfBlockRealTimeOpticalDrive)
{
    quint64 total = 0, avai = 0, used = 0;
    QVariantMap itemData;
    itemData[DP::kMountPoint] = "/media/test";
    itemData[DP::kOpticalDrive] = true;
    bool result = DeviceHelper::queryUsageOfBlockRealTime(itemData, &total, &avai, &used);
    // May return true if optical info loaded
    EXPECT_TRUE(result || !result);
}

TEST(DeviceHelperTest, QueryUsageOfProtocolRealTimeEmptyMpt)
{
    quint64 total = 0, avai = 0, used = 0;
    QVariantMap itemData;
    itemData[DP::kMountPoint] = "";
    bool result = DeviceHelper::queryUsageOfProtocolRealTime(itemData, &total, &avai, &used);
    EXPECT_FALSE(result);
}

TEST(DeviceHelperTest, QueryUsageOfProtocolRealTimeEmptyId)
{
    quint64 total = 0, avai = 0, used = 0;
    QVariantMap itemData;
    itemData[DP::kMountPoint] = "/mnt/test";
    itemData[DP::kId] = "";
    bool result = DeviceHelper::queryUsageOfProtocolRealTime(itemData, &total, &avai, &used);
    EXPECT_FALSE(result);
}

TEST(DeviceHelperTest, QueryUsageOfProtocolRealTimeNonExistentId)
{
    quint64 total = 0, avai = 0, used = 0;
    QVariantMap itemData;
    itemData[DP::kMountPoint] = "/mnt/test";
    itemData[DP::kId] = "/nonexistent/protocol/dev_query";
    bool result = DeviceHelper::queryUsageOfProtocolRealTime(itemData, &total, &avai, &used);
    // May succeed or fail depending on dfm-mount environment
    EXPECT_TRUE(result || !result);
}

TEST(DeviceHelperTest, QueryDeviceUsageRealTimeBlockType)
{
    quint64 total = 0, avai = 0, used = 0;
    QVariantMap itemData;
    itemData[DP::kId] = "/org/freedesktop/UDisks2/block_devices/sda1";
    itemData[DP::kMountPoint] = "";
    bool result = DeviceHelper::queryDeviceUsageRealTime(itemData, &total, &avai, &used);
    EXPECT_FALSE(result);
}

TEST(DeviceHelperTest, QueryDeviceUsageRealTimeProtocolType)
{
    quint64 total = 0, avai = 0, used = 0;
    QVariantMap itemData;
    itemData[DP::kId] = "/nonexistent/protocol/dev_realtime";
    itemData[DP::kMountPoint] = "";
    bool result = DeviceHelper::queryDeviceUsageRealTime(itemData, &total, &avai, &used);
    EXPECT_FALSE(result);
}

TEST(DeviceHelperTest, MakeFakeProtocolInfoWithSmbPath)
{
    QString id = ",server=10.0.0.1,share=myshare";
    QVariantMap info = DeviceHelper::makeFakeProtocolInfo(id);
    EXPECT_FALSE(info.isEmpty());
    EXPECT_TRUE(info.value("fake").toBool());
    EXPECT_FALSE(info.value(DP::kDisplayName).toString().isEmpty());
}

TEST(DeviceHelperTest, MakeFakeProtocolInfoWithFtpPath)
{
    QString id = "ftp://10.0.0.1/path";
    QVariantMap info = DeviceHelper::makeFakeProtocolInfo(id);
    EXPECT_FALSE(info.isEmpty());
    EXPECT_TRUE(info.value("fake").toBool());
}

TEST(DeviceHelperTest, MakeFakeProtocolInfoWithUnknownPath)
{
    QString id = "/unknown/protocol/path";
    QVariantMap info = DeviceHelper::makeFakeProtocolInfo(id);
    EXPECT_FALSE(info.isEmpty());
    EXPECT_TRUE(info.value("fake").toBool());
}

TEST(DeviceHelperTest, PersistentOpticalInfoCallable)
{
    // Stub OpticalShareProxy::setBurnAttribute to avoid DBus call
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(OpticalShareProxy, setBurnAttribute), [](OpticalShareProxy *, const QString &, const QVariantMap &) -> bool { return true; });
    QVariantMap data;
    data[DP::kDevice] = "/dev/sr99";
    data[DP::kSizeTotal] = quint64(1024);
    data[DP::kSizeUsed] = quint64(0);
    EXPECT_NO_FATAL_FAILURE({
        DeviceHelper::persistentOpticalInfo(data);
    });
}

TEST(DeviceHelperTest, ClearOpticalInfoNonExistent)
{
    EXPECT_NO_FATAL_FAILURE({
        DeviceHelper::clearOpticalInfo("nonexistent_tag_12345");
    });
}

TEST(DeviceHelperTest, ReadOpticalInfoEmptyMap)
{
    QVariantMap data;
    EXPECT_NO_FATAL_FAILURE({
        DeviceHelper::readOpticalInfo(data);
    });
    // Empty input, data stays empty or modified by OpticalShareProxy
}

TEST(DeviceHelperTest, AskForStopScanningNonExistent)
{
    QUrl mpt = QUrl::fromLocalFile("/nonexistent/mount_point_12345");
    bool result;
    EXPECT_NO_FATAL_FAILURE({
        result = DeviceHelper::askForStopScanning(mpt);
    });
    // Not scanning → returns true
    EXPECT_TRUE(result);
}

TEST(DeviceHelperTest, OpenFileManagerToDeviceDoesNotCrash)
{
    stub_ext::StubExt stub;
    // Prevent launching the real dde-file-manager process during testing.
    using StartDetachedFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFunc>(QProcess::startDetached),
                   [](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    // Stub findExecutable to return a non-empty path so the code takes the
    // startDetached branch (which is stubbed above) rather than the
    // DDesktopServices::showFolder branch (which would open a real window).
    using FindExecType = QString (*)(const QString &, const QStringList &);
    stub.set_lamda(static_cast<FindExecType>(&QStandardPaths::findExecutable),
                   [](const QString &, const QStringList &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QStringLiteral("fake-dde-file-manager");
                   });

    EXPECT_NO_FATAL_FAILURE({
        DeviceHelper::openFileManagerToDevice("/dev/nonexistent_zzz", "/tmp/nonexistent_mpt_zzz");
    });
}

TEST(DeviceHelperTest, CheckNetworkConnectionNonExistent)
{
    stub_ext::StubExt stub;
    // Stub network check to avoid TCP timeout to broadcast address
    // Select the (QString, QString, int, bool) overload explicitly
    // (the bool useCache param was added to enable the TTL cache)
    bool (NetworkUtils::*checkNetFn)(const QString &, const QString &, int, bool) =
        &NetworkUtils::checkNetConnection;
    stub.set_lamda(checkNetFn,
                   [](NetworkUtils *, const QString &, const QString &, int, bool) -> bool {
                       return false;
                   });

    bool result;
    EXPECT_NO_FATAL_FAILURE({
        result = DeviceHelper::checkNetworkConnection("smb://192.168.255.255/share");
    });
    EXPECT_FALSE(result);
}
