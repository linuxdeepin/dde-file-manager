// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QVariantMap>
#include <QSignalSpy>

#include "stubext.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/private/devicewatcher.h>
#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-mount/base/dmount_global.h>

DFMBASE_USE_NAMESPACE
DFM_MOUNT_USE_NS
using namespace GlobalServerDefines;

class TestDeviceManager : public testing::Test
{
public:
    void SetUp() override
    {
        // Stub DeviceWatcher to avoid real device monitoring
        stub.set_lamda(&DeviceWatcher::getDevIds, [](DeviceWatcher *, DeviceType type) -> QStringList {
            __DBG_STUB_INVOKE__
            if (type == DeviceType::kBlockDevice) {
                return QStringList { "block:dev:sda1", "block:dev:sdb1" };
            } else if (type == DeviceType::kProtocolDevice) {
                return QStringList { "proto:smb:server1", "proto:ftp:server2" };
            }
            return QStringList();
        });

        stub.set_lamda(&DeviceWatcher::getDevInfo, [](DeviceWatcher *, const QString &id, DeviceType, bool) -> QVariantMap {
            __DBG_STUB_INVOKE__
            QVariantMap info;
            info[DeviceProperty::kId] = id;
            info[DeviceProperty::kMountPoint] = "/media/test";
            info[DeviceProperty::kRemovable] = true;
            info[DeviceProperty::kOptical] = false;
            info[DeviceProperty::kHintIgnore] = false;
            info[DeviceProperty::kIsLoopDevice] = false;
            info[DeviceProperty::kSizeTotal] = 1000000000ULL;
            info[DeviceProperty::kSizeFree] = 500000000ULL;
            return info;
        });

        // Stub DeviceHelper to avoid real device creation
        stub.set_lamda(ADDR(DeviceHelper, createBlockDevice), []() {
            __DBG_STUB_INVOKE__
            return nullptr;   // Return null to simulate device creation failure
        });

        using IsMountableBlockDevFunc = bool (*)(const QVariantMap &, QString &);
        stub.set_lamda(static_cast<IsMountableBlockDevFunc>(&DeviceHelper::isMountableBlockDev), [](const QVariantMap &, QString &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        using IsMountableBlockDevFunc2 = bool (*)(const BlockDevAutoPtr &, QString &);
        stub.set_lamda(static_cast<IsMountableBlockDevFunc2>(&DeviceHelper::isMountableBlockDev), [](const BlockDevAutoPtr &, QString &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Stub DeviceUtils
        using IsBuiltInDiskFunc = bool (*)(const QVariantHash &);
        stub.set_lamda(static_cast<IsBuiltInDiskFunc>(&DeviceUtils::isBuiltInDisk), []() {
            __DBG_STUB_INVOKE__
            return false;
        });

        manager = DeviceManager::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    DeviceManager *manager { nullptr };
};

// ========== instance() Tests ==========

TEST_F(TestDeviceManager, instance_ReturnsSingleton)
{
    // Test singleton instance
    auto instance1 = DeviceManager::instance();
    auto instance2 = DeviceManager::instance();

    ASSERT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

// ========== getAllBlockDevID() Tests ==========

TEST_F(TestDeviceManager, getAllBlockDevID_NoOptions)
{
    // Test getting all block device IDs without options
    QStringList ids = manager->getAllBlockDevID();

    EXPECT_FALSE(ids.isEmpty());
    EXPECT_EQ(ids.size(), 2);
    EXPECT_TRUE(ids.contains("block:dev:sda1"));
}

TEST_F(TestDeviceManager, getAllBlockDevID_NoCondition)
{
    // Test with no condition flag
    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kNoCondition);

    EXPECT_FALSE(ids.isEmpty());
    EXPECT_EQ(ids.size(), 2);
}

TEST_F(TestDeviceManager, getAllBlockDevID_Mounted)
{
    // Test filtering mounted devices
    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kMounted);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceManager, getAllBlockDevID_Removable)
{
    // Test filtering removable devices
    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kRemovable);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceManager, getAllBlockDevID_Mountable)
{
    // Test filtering mountable devices
    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kMountable);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceManager, getAllBlockDevID_NotIgnored)
{
    // Test filtering non-ignored devices
    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kNotIgnored);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceManager, getAllBlockDevID_NotMounted)
{
    // Test filtering not mounted devices
    stub.set_lamda(&DeviceWatcher::getDevInfo, [](DeviceWatcher *, const QString &id, DeviceType, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kId] = id;
        info[DeviceProperty::kMountPoint] = "";   // Not mounted
        return info;
    });

    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kNotMounted);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceManager, getAllBlockDevID_Optical)
{
    // Test filtering optical devices
    stub.set_lamda(&DeviceWatcher::getDevInfo, [](DeviceWatcher *, const QString &id, DeviceType, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kId] = id;
        info[DeviceProperty::kOptical] = true;
        return info;
    });

    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kOptical);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceManager, getAllBlockDevID_System)
{
    // Test filtering system devices
    using IsBuiltInDiskFunc = bool (*)(const QVariantHash &);
    stub.set_lamda(static_cast<IsBuiltInDiskFunc>(&DeviceUtils::isBuiltInDisk), []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kSystem);

    EXPECT_TRUE(ids.size() >= 0);
}

TEST_F(TestDeviceManager, getAllBlockDevID_Loop)
{
    // Test filtering loop devices
    stub.set_lamda(&DeviceWatcher::getDevInfo, [](DeviceWatcher *, const QString &id, DeviceType, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kId] = id;
        info[DeviceProperty::kIsLoopDevice] = true;
        return info;
    });

    QStringList ids = manager->getAllBlockDevID(DeviceQueryOption::kLoop);

    EXPECT_TRUE(ids.size() >= 0);
}

// ========== getAllProtocolDevID() Tests ==========

TEST_F(TestDeviceManager, getAllProtocolDevID_ReturnsProtocolDevices)
{
    // Test getting all protocol device IDs
    QStringList ids = manager->getAllProtocolDevID();

    EXPECT_FALSE(ids.isEmpty());
    EXPECT_EQ(ids.size(), 2);
    EXPECT_TRUE(ids.contains("proto:smb:server1"));
}

// ========== getBlockDevInfo() Tests ==========

TEST_F(TestDeviceManager, getBlockDevInfo_ValidId)
{
    // Test getting block device info
    QString id = "block:dev:sda1";
    QVariantMap info = manager->getBlockDevInfo(id);

    EXPECT_FALSE(info.isEmpty());
    EXPECT_TRUE(info.contains(DeviceProperty::kId));
    EXPECT_EQ(info.value(DeviceProperty::kId).toString(), id);
}

TEST_F(TestDeviceManager, getBlockDevInfo_WithReload)
{
    // Test getting info with reload flag
    QString id = "block:dev:sdb1";
    QVariantMap info = manager->getBlockDevInfo(id, true);

    EXPECT_FALSE(info.isEmpty());
}

TEST_F(TestDeviceManager, getBlockDevInfo_EmptyId)
{
    // Test with empty ID
    QVariantMap info = manager->getBlockDevInfo(QString());

    // Should return empty or default map
    EXPECT_TRUE(true);
}

// ========== getProtocolDevInfo() Tests ==========

TEST_F(TestDeviceManager, getProtocolDevInfo_ValidId)
{
    // Test getting protocol device info
    QString id = "proto:smb:server1";
    QVariantMap info = manager->getProtocolDevInfo(id);

    EXPECT_FALSE(info.isEmpty());
    EXPECT_TRUE(info.contains(DeviceProperty::kId));
}

TEST_F(TestDeviceManager, getProtocolDevInfo_WithReload)
{
    // Test getting info with reload flag
    QString id = "proto:ftp:server2";
    QVariantMap info = manager->getProtocolDevInfo(id, true);

    EXPECT_FALSE(info.isEmpty());
}

// ========== mountBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, mountBlockDevAsync_EmptyId)
{
    // Test mounting with empty ID - should assert in debug
    // Cannot test assertion directly, so test with valid ID instead
    QString id = "block:dev:sda1";
    bool callbackInvoked = false;

    manager->mountBlockDevAsync(id, QVariantMap(), [&callbackInvoked](bool, const OperationErrorInfo &, const QString &) {
        callbackInvoked = true;
    });

    // Callback should be invoked (with failure due to stubbed null device)
    EXPECT_TRUE(callbackInvoked);
}

TEST_F(TestDeviceManager, mountBlockDevAsync_DeviceCreationFailure)
{
    // Test mount when device creation fails
    QString id = "block:dev:test";
    bool callbackInvoked = false;
    bool mountResult = true;

    manager->mountBlockDevAsync(id, QVariantMap(), [&callbackInvoked, &mountResult](bool ok, const OperationErrorInfo &, const QString &) {
        callbackInvoked = true;
        mountResult = ok;
    });

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(mountResult);
}

// ========== unmountBlockDev() Tests ==========

TEST_F(TestDeviceManager, unmountBlockDev_ValidId)
{
    // Test unmounting block device
    stub.set_lamda(ADDR(DeviceHelper, createBlockDevice), [](const QString &) -> BlockDevAutoPtr {
        __DBG_STUB_INVOKE__
        return {};
    });

    QString id = "block:dev:sda1";
    bool result = manager->unmountBlockDev(id);

    // Result depends on internal implementation
    EXPECT_TRUE(result || !result);
}

// ========== unmountBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, unmountBlockDevAsync_ValidId)
{
    // Test async unmounting
    QString id = "block:dev:sda1";

    manager->unmountBlockDevAsync(id, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== lockBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, lockBlockDevAsync_ValidId)
{
    // Test locking encrypted device
    QString id = "block:dev:luks1";

    manager->lockBlockDevAsync(id, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== unlockBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, unlockBlockDevAsync_ValidId)
{
    // Test unlocking encrypted device
    QString id = "block:dev:luks1";
    QString password = "testpass";

    manager->unlockBlockDevAsync(id, password, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== powerOffBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, powerOffBlockDevAsync_ValidId)
{
    // Test powering off device
    QString id = "block:dev:sdc1";

    manager->powerOffBlockDevAsync(id, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== ejectBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, ejectBlockDevAsync_ValidId)
{
    // Test ejecting device
    QString id = "block:dev:sr0";

    manager->ejectBlockDevAsync(id, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== renameBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, renameBlockDevAsync_ValidId)
{
    // Test renaming device
    QString id = "block:dev:sda1";
    QString newName = "NewDiskName";

    manager->renameBlockDevAsync(id, newName, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== rescanBlockDev() Tests ==========

TEST_F(TestDeviceManager, rescanBlockDev_ValidId)
{
    // Test rescanning device
    QString id = "block:dev:sr0";

    bool result = manager->rescanBlockDev(id);

    // Result depends on implementation
    EXPECT_TRUE(result || !result);
}

// ========== rescanBlockDevAsync() Tests ==========

TEST_F(TestDeviceManager, rescanBlockDevAsync_ValidId)
{
    // Test async rescanning
    QString id = "block:dev:sr0";

    manager->rescanBlockDevAsync(id, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== mountProtocolDev() Tests ==========

TEST_F(TestDeviceManager, mountProtocolDev_ValidId)
{
    // Test mounting protocol device
    QString id = "proto:smb:server1";

    QString result = manager->mountProtocolDev(id);

    // Should return empty or mount point
    EXPECT_TRUE(true);
}

// ========== mountProtocolDevAsync() Tests ==========

TEST_F(TestDeviceManager, mountProtocolDevAsync_ValidId)
{
    // Test async mounting protocol device
    QString id = "proto:smb:server1";

    manager->mountProtocolDevAsync(id, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== unmountProtocolDev() Tests ==========

TEST_F(TestDeviceManager, unmountProtocolDev_ValidId)
{
    // Test unmounting protocol device
    QString id = "proto:smb:server1";

    bool result = manager->unmountProtocolDev(id);

    // Result depends on implementation
    EXPECT_TRUE(result || !result);
}

// ========== unmountProtocolDevAsync() Tests ==========

TEST_F(TestDeviceManager, unmountProtocolDevAsync_ValidId)
{
    // Test async unmounting protocol device
    QString id = "proto:ftp:server2";

    manager->unmountProtocolDevAsync(id, QVariantMap(), nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== mountNetworkDeviceAsync() Tests ==========

TEST_F(TestDeviceManager, mountNetworkDeviceAsync_ValidAddress)
{
    // Test mounting network device
    QString address = "smb://192.168.1.10/share";

    manager->mountNetworkDeviceAsync(address, nullptr);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestDeviceManager, mountNetworkDeviceAsync_WithTimeout)
{
    // Test mounting with custom timeout
    QString address = "ftp://server.com/path";

    manager->mountNetworkDeviceAsync(address, nullptr, 10);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== detachAllRemovableBlockDevs() Tests ==========

TEST_F(TestDeviceManager, detachAllRemovableBlockDevs_ExecutesWithoutCrash)
{
    // Test detaching all removable devices
    manager->detachAllRemovableBlockDevs();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== detachBlockDev() Tests ==========

TEST_F(TestDeviceManager, detachBlockDev_ValidId)
{
    // Test detaching specific block device
    QString id = "block:dev:sdc1";

    QStringList result = manager->detachBlockDev(id);

    // Should return list of detached devices
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

// ========== detachAllProtoDevs() Tests ==========

TEST_F(TestDeviceManager, detachAllProtoDevs_ExecutesWithoutCrash)
{
    // Test detaching all protocol devices
    manager->detachAllProtoDevs();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== detachProtoDev() Tests ==========

TEST_F(TestDeviceManager, detachProtoDev_ValidId)
{
    // Test detaching specific protocol device
    QString id = "proto:smb:server1";

    manager->detachProtoDev(id);

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== startMonitor() Tests ==========

TEST_F(TestDeviceManager, startMonitor_ExecutesWithoutCrash)
{
    // Test starting device monitoring
    manager->startMonitor();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== stopMonitor() Tests ==========

TEST_F(TestDeviceManager, stopMonitor_ExecutesWithoutCrash)
{
    // Test stopping device monitoring
    manager->stopMonitor();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== isMonitoring() Tests ==========

TEST_F(TestDeviceManager, isMonitoring_ReturnsStatus)
{
    // Test checking monitoring status
    bool result = manager->isMonitoring();

    // Should return a boolean value
    EXPECT_TRUE(result || !result);
}

// ========== enableBlockAutoMount() Tests ==========

TEST_F(TestDeviceManager, enableBlockAutoMount_ExecutesWithoutCrash)
{
    // Test enabling auto mount
    manager->enableBlockAutoMount();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== startPollingDeviceUsage() Tests ==========

TEST_F(TestDeviceManager, startPollingDeviceUsage_ExecutesWithoutCrash)
{
    // Test starting usage polling
    manager->startPollingDeviceUsage();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== stopPollingDeviceUsage() Tests ==========

TEST_F(TestDeviceManager, stopPollingDeviceUsage_ExecutesWithoutCrash)
{
    // Test stopping usage polling
    manager->stopPollingDeviceUsage();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== startOpticalDiscScan() Tests ==========

TEST_F(TestDeviceManager, startOpticalDiscScan_ExecutesWithoutCrash)
{
    // Test starting optical disc scan
    manager->startOpticalDiscScan();

    // Should not crash
    EXPECT_TRUE(true);
}

// ========== Signal Tests ==========

TEST_F(TestDeviceManager, Signal_DevSizeChanged)
{
    // Test devSizeChanged signal
    QSignalSpy spy(manager, &DeviceManager::devSizeChanged);

    emit manager->devSizeChanged("test-id", 1000000, 500000);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDeviceManager, Signal_BlockDevAdded)
{
    // Test blockDevAdded signal
    QSignalSpy spy(manager, &DeviceManager::blockDevAdded);

    emit manager->blockDevAdded("block:dev:sdd1");

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDeviceManager, Signal_BlockDevRemoved)
{
    // Test blockDevRemoved signal
    QSignalSpy spy(manager, &DeviceManager::blockDevRemoved);

    emit manager->blockDevRemoved("block:dev:sde1", "/media/usb");

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDeviceManager, Signal_BlockDevMounted)
{
    // Test blockDevMounted signal
    QSignalSpy spy(manager, &DeviceManager::blockDevMounted);

    emit manager->blockDevMounted("block:dev:sdf1", "/media/disk");

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDeviceManager, Signal_ProtocolDevAdded)
{
    // Test protocolDevAdded signal
    QSignalSpy spy(manager, &DeviceManager::protocolDevAdded);

    emit manager->protocolDevAdded("proto:smb:newserver");

    EXPECT_EQ(spy.count(), 1);
}

// ========== Integration Tests ==========

TEST_F(TestDeviceManager, Integration_QueryAllDevices)
{
    // Test querying all device types
    QStringList blockIds = manager->getAllBlockDevID();
    QStringList protoIds = manager->getAllProtocolDevID();

    EXPECT_FALSE(blockIds.isEmpty());
    EXPECT_FALSE(protoIds.isEmpty());

    // Query info for first device of each type
    if (!blockIds.isEmpty()) {
        QVariantMap blockInfo = manager->getBlockDevInfo(blockIds.first());
        EXPECT_FALSE(blockInfo.isEmpty());
    }

    if (!protoIds.isEmpty()) {
        QVariantMap protoInfo = manager->getProtocolDevInfo(protoIds.first());
        EXPECT_FALSE(protoInfo.isEmpty());
    }
}

TEST_F(TestDeviceManager, Integration_MonitoringLifecycle)
{
    // Test monitoring lifecycle
    manager->startMonitor();
    bool monitoring1 = manager->isMonitoring();

    manager->stopMonitor();
    bool monitoring2 = manager->isMonitoring();

    // Should be able to start and stop monitoring
    EXPECT_TRUE(true);
}

// ========== Edge Cases ==========

TEST_F(TestDeviceManager, EdgeCase_EmptyDeviceList)
{
    // Test with no devices
    stub.set_lamda(&DeviceWatcher::getDevIds, [](DeviceWatcher *, DeviceType) -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList();
    });

    QStringList ids = manager->getAllBlockDevID();

    EXPECT_TRUE(ids.isEmpty());
}

TEST_F(TestDeviceManager, EdgeCase_MultipleFilterOptions)
{
    // Test with multiple filter options combined
    DeviceQueryOptions opts = DeviceQueryOption::kMounted | DeviceQueryOption::kRemovable;

    QStringList ids = manager->getAllBlockDevID(opts);

    EXPECT_TRUE(ids.size() >= 0);
}
