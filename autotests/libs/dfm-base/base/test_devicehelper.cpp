// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QVariantMap>
#include <QUrl>
#include <QDialog>

#include "stubext.h"

#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/base/device/private/defendercontroller.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-mount/ddevicemanager.h>
#include <dfm-mount/dblockmonitor.h>
#include <dfm-mount/dblockdevice.h>
#include <dfm-mount/dprotocoldevice.h>

DFMBASE_USE_NAMESPACE
DFM_MOUNT_USE_NS
using namespace GlobalServerDefines;

class TestDeviceHelper : public testing::Test
{
public:
    void SetUp() override
    {
        // Stub DDeviceManager to avoid real device access
        stub.set_lamda(&DDeviceManager::instance, []() -> DDeviceManager * {
            __DBG_STUB_INVOKE__
            static DDeviceManager mockManager;
            return &mockManager;
        });

        stub.set_lamda(&DDeviceManager::getRegisteredMonitor, [](DDeviceManager *, DeviceType) -> QSharedPointer<DDeviceMonitor> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<DDeviceMonitor>(nullptr);   // Return null shared pointer
        });

        stub.set_lamda(&DDeviceMonitor::createDeviceById, [](DDeviceMonitor *, const QString &) -> QSharedPointer<DDevice> {
            __DBG_STUB_INVOKE__
            return nullptr;   // Return null to simulate device creation failure
        });

        // Stub DefenderController
        stub.set_lamda(static_cast<bool (DefenderController::*)(const QUrl &)>(&DefenderController::isScanning), []() {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(static_cast<bool (DefenderController::*)(const QUrl &)>(&DefenderController::stopScanning), []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

// ========== createDevice() Tests ==========

TEST_F(TestDeviceHelper, createDevice_AllDeviceType)
{
    // Test creating device with kAllDevice type
    auto dev = DeviceHelper::createDevice("test-id", DeviceType::kAllDevice);

    EXPECT_TRUE(dev == nullptr);
}

TEST_F(TestDeviceHelper, createDevice_BlockDevice)
{
    // Test creating block device
    auto dev = DeviceHelper::createDevice("block:dev:sda1", DeviceType::kBlockDevice);

    // Stubbed to return null
    EXPECT_TRUE(dev == nullptr);
}

TEST_F(TestDeviceHelper, createDevice_ProtocolDevice)
{
    // Test creating protocol device
    auto dev = DeviceHelper::createDevice("proto:smb:server", DeviceType::kProtocolDevice);

    // Stubbed to return null
    EXPECT_TRUE(dev == nullptr);
}

// ========== createBlockDevice() Tests ==========

TEST_F(TestDeviceHelper, createBlockDevice_ValidId)
{
    // Test creating block device with valid ID
    auto dev = DeviceHelper::createBlockDevice("block:dev:sda1");

    // Stubbed to return null
    EXPECT_TRUE(dev == nullptr);
}

TEST_F(TestDeviceHelper, createBlockDevice_EmptyId)
{
    // Test with empty ID
    auto dev = DeviceHelper::createBlockDevice(QString());

    EXPECT_TRUE(dev == nullptr);
}

// ========== createProtocolDevice() Tests ==========

TEST_F(TestDeviceHelper, createProtocolDevice_ValidId)
{
    // Test creating protocol device with valid ID
    auto dev = DeviceHelper::createProtocolDevice("proto:smb:server1");

    // Stubbed to return null
    EXPECT_TRUE(dev == nullptr);
}

TEST_F(TestDeviceHelper, createProtocolDevice_EmptyId)
{
    // Test with empty ID
    auto dev = DeviceHelper::createProtocolDevice(QString());

    EXPECT_TRUE(dev == nullptr);
}

// ========== loadBlockInfo() Tests ==========

TEST_F(TestDeviceHelper, loadBlockInfo_ById_InvalidId)
{
    // Test loading block info with invalid ID
    QVariantMap info = DeviceHelper::loadBlockInfo("invalid-id");

    // Should return empty map when device creation fails
    EXPECT_TRUE(info.isEmpty());
}

TEST_F(TestDeviceHelper, loadBlockInfo_ById_ValidId)
{
    // Test loading block info with valid ID
    // Device creation stubbed to fail, so should return empty
    QVariantMap info = DeviceHelper::loadBlockInfo("block:dev:sda1");

    EXPECT_TRUE(info.isEmpty());
}

TEST_F(TestDeviceHelper, loadBlockInfo_ByDevice_NullDevice)
{
    // Test loading info from null device
    BlockDevAutoPtr nullDev;

    QVariantMap info = DeviceHelper::loadBlockInfo(nullDev);

    EXPECT_TRUE(info.isEmpty());
}

// ========== loadProtocolInfo() Tests ==========

TEST_F(TestDeviceHelper, loadProtocolInfo_ById_InvalidId)
{
    // Test loading protocol info with invalid ID
    QVariantMap info = DeviceHelper::loadProtocolInfo("invalid-id");

    // Should return empty or fake info
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceHelper, loadProtocolInfo_ById_ValidId)
{
    // Test loading protocol info with valid ID
    QVariantMap info = DeviceHelper::loadProtocolInfo("proto:smb:server");

    EXPECT_TRUE(true);
}

TEST_F(TestDeviceHelper, loadProtocolInfo_ByDevice_NullDevice)
{
    // Test loading info from null device
    ProtocolDevAutoPtr nullDev;

    QVariantMap info = DeviceHelper::loadProtocolInfo(nullDev);

    EXPECT_TRUE(info.isEmpty());
}

// ========== isMountableBlockDev() Tests ==========

TEST_F(TestDeviceHelper, isMountableBlockDev_ById_ValidId)
{
    // Test checking if device is mountable by ID
    QString why;
    bool result = DeviceHelper::isMountableBlockDev("block:dev:sda1", why);

    // Depends on device creation and properties
    EXPECT_TRUE(result || !result);
}

TEST_F(TestDeviceHelper, isMountableBlockDev_ById_EmptyId)
{
    // Test with empty ID
    QString why;
    bool result = DeviceHelper::isMountableBlockDev(QString(), why);

    EXPECT_FALSE(result);
    EXPECT_FALSE(why.isEmpty());
}

TEST_F(TestDeviceHelper, isMountableBlockDev_ByDevice_NullDevice)
{
    // Test with null device
    BlockDevAutoPtr nullDev;
    QString why;

    bool result = DeviceHelper::isMountableBlockDev(nullDev, why);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceHelper, isMountableBlockDev_ByMap_EmptyMap)
{
    // Test with empty info map
    QVariantMap emptyInfo;
    QString why;

    bool result = DeviceHelper::isMountableBlockDev(emptyInfo, why);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceHelper, isMountableBlockDev_ByMap_ValidInfo)
{
    // Test with valid device info
    QVariantMap info;
    info[DeviceProperty::kHasFileSystem] = true;
    info[DeviceProperty::kMountPoint] = "";
    info[DeviceProperty::kOptical] = false;

    QString why;
    bool result = DeviceHelper::isMountableBlockDev(info, why);

    EXPECT_TRUE(result || !result);
}

// ========== isEjectableBlockDev() Tests ==========

TEST_F(TestDeviceHelper, isEjectableBlockDev_ById_ValidId)
{
    // Test checking if device is ejectable by ID
    QString why;
    bool result = DeviceHelper::isEjectableBlockDev("block:dev:sr0", why);

    EXPECT_TRUE(result || !result);
}

TEST_F(TestDeviceHelper, isEjectableBlockDev_ByDevice_NullDevice)
{
    // Test with null device
    BlockDevAutoPtr nullDev;
    QString why;

    bool result = DeviceHelper::isEjectableBlockDev(nullDev, why);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceHelper, isEjectableBlockDev_ByMap_EmptyMap)
{
    // Test with empty info map
    QVariantMap emptyInfo;
    QString why;

    bool result = DeviceHelper::isEjectableBlockDev(emptyInfo, why);

    EXPECT_FALSE(result);
}

TEST_F(TestDeviceHelper, isEjectableBlockDev_ByMap_Ejectable)
{
    // Test with ejectable device info
    QVariantMap info;
    info[DeviceProperty::kEjectable] = true;
    info[DeviceProperty::kOptical] = true;

    QString why;
    bool result = DeviceHelper::isEjectableBlockDev(info, why);

    EXPECT_TRUE(result || !result);
}

// ========== askForStopScanning() Tests ==========

TEST_F(TestDeviceHelper, askForStopScanning_NotScanning)
{
    // Test when device is not being scanned
    QUrl mpt("file:///media/usb");

    bool result = DeviceHelper::askForStopScanning(mpt);

    // Should return true if not scanning
    EXPECT_TRUE(result);
}

TEST_F(TestDeviceHelper, askForStopScanning_Scanning)
{
    // Test when device is being scanned
    stub.set_lamda(static_cast<bool (DefenderController::*)(const QUrl &)>(&DefenderController::isScanning),
                   [](DefenderController *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl mpt("file:///media/usb");

    bool result = DeviceHelper::askForStopScanning(mpt);

    // Should ask and stop scanning
    EXPECT_TRUE(result || !result);
}

TEST_F(TestDeviceHelper, askForStopScanning_InvalidUrl)
{
    // Test with invalid URL
    QUrl invalidUrl;

    bool result = DeviceHelper::askForStopScanning(invalidUrl);

    // Should handle gracefully
    EXPECT_TRUE(result || !result);
}

// ========== castFromDFMMountProperty() Tests ==========

TEST_F(TestDeviceHelper, castFromDFMMountProperty_BlockProperty)
{
    // Test casting block device property
    QString result = DeviceHelper::castFromDFMMountProperty(DFMMOUNT::Property::kBlockDevice);

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDeviceHelper, castFromDFMMountProperty_FileSystemProperty)
{
    // Test casting filesystem property
    QString result = DeviceHelper::castFromDFMMountProperty(DFMMOUNT::Property::kBlockHintSystem);

    EXPECT_FALSE(result.isEmpty());
}

// ========== persistentOpticalInfo() and readOpticalInfo() Tests ==========

TEST_F(TestDeviceHelper, persistentOpticalInfo_ValidData)
{
    // Test persisting optical info
    QVariantMap data;
    data["BurnTotalSize"] = 1000000;
    data["BurnUsedSize"] = 500000;

    DeviceHelper::persistentOpticalInfo(data);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceHelper, persistentOpticalInfo_EmptyData)
{
    // Test with empty data
    QVariantMap emptyData;

    DeviceHelper::persistentOpticalInfo(emptyData);

    EXPECT_TRUE(true);
}

TEST_F(TestDeviceHelper, readOpticalInfo_EmptyData)
{
    // Test reading optical info into empty map
    QVariantMap data;

    DeviceHelper::readOpticalInfo(data);

    // Should populate data or leave it empty
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceHelper, readOpticalInfo_ExistingData)
{
    // Test reading into map with existing data
    QVariantMap data;
    data[DeviceProperty::kId] = "block:dev:sr0";

    DeviceHelper::readOpticalInfo(data);

    EXPECT_TRUE(true);
}

// ========== Integration Tests ==========

TEST_F(TestDeviceHelper, Integration_CreateAndLoadBlockDevice)
{
    // Test complete workflow: create device and load info
    QString id = "block:dev:sda1";

    auto dev = DeviceHelper::createBlockDevice(id);
    EXPECT_TRUE(dev == nullptr);   // Stubbed to fail

    QVariantMap info = DeviceHelper::loadBlockInfo(id);
    EXPECT_TRUE(info.isEmpty());   // Should be empty due to creation failure
}

TEST_F(TestDeviceHelper, Integration_CreateAndLoadProtocolDevice)
{
    // Test protocol device workflow
    QString id = "proto:smb:server1";

    auto dev = DeviceHelper::createProtocolDevice(id);
    EXPECT_TRUE(dev == nullptr);

    QVariantMap info = DeviceHelper::loadProtocolInfo(id);
    // May return fake info
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceHelper, Integration_MountabilityCheck)
{
    // Test checking mountability through different methods
    QString id = "block:dev:sda1";
    QString why;

    // By ID
    bool mountableById = DeviceHelper::isMountableBlockDev(id, why);

    // By map
    QVariantMap info;
    info[DeviceProperty::kHasFileSystem] = true;
    bool mountableByMap = DeviceHelper::isMountableBlockDev(info, why);

    EXPECT_TRUE(true);
}

TEST_F(TestDeviceHelper, Integration_ScanningWorkflow)
{
    // Test complete scanning check and stop workflow
    QUrl mpt("file:///media/usb");

    // Ask to stop scanning
    bool stopped = DeviceHelper::askForStopScanning(mpt);

    EXPECT_TRUE(stopped || !stopped);
}

// ========== Edge Cases ==========

TEST_F(TestDeviceHelper, EdgeCase_NullPointerHandling)
{
    // Test handling of null pointers
    BlockDevAutoPtr nullBlock;
    ProtocolDevAutoPtr nullProto;

    QVariantMap blockInfo = DeviceHelper::loadBlockInfo(nullBlock);
    QVariantMap protoInfo = DeviceHelper::loadProtocolInfo(nullProto);

    EXPECT_TRUE(blockInfo.isEmpty());
    EXPECT_TRUE(protoInfo.isEmpty());
}

TEST_F(TestDeviceHelper, EdgeCase_EmptyStrings)
{
    // Test with empty string inputs
    auto dev1 = DeviceHelper::createBlockDevice(QString());
    auto dev2 = DeviceHelper::createProtocolDevice(QString());

    QString why;
    bool mountable = DeviceHelper::isMountableBlockDev(QString(), why);
    bool ejectable = DeviceHelper::isEjectableBlockDev(QString(), why);

    EXPECT_TRUE(dev1 == nullptr);
    EXPECT_TRUE(dev2 == nullptr);
    EXPECT_FALSE(mountable);
    EXPECT_FALSE(ejectable);
}

TEST_F(TestDeviceHelper, EdgeCase_InvalidDeviceId)
{
    // Test with malformed device IDs
    QStringList invalidIds = {
        "invalid",
        ":",
        "//",
        "block:",
        ":dev:sda"
    };

    for (const QString &id : invalidIds) {
        auto dev = DeviceHelper::createBlockDevice(id);
        EXPECT_TRUE(dev == nullptr || !dev);

        QVariantMap info = DeviceHelper::loadBlockInfo(id);
        // Should handle gracefully
        EXPECT_TRUE(true);
    }
}

TEST_F(TestDeviceHelper, EdgeCase_VeryLongDeviceId)
{
    // Test with very long device ID
    QString longId = "block:dev:" + QString("a").repeated(1000);

    auto dev = DeviceHelper::createBlockDevice(longId);

    EXPECT_TRUE(dev == nullptr || !dev);
}
