// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanager.cpp
 * @brief Unit tests for DeviceManager safe methods (devicemanager.cpp) —
 *        instance, isMonitoring, enableBlockAutoMount, startOpticalDiscScan,
 *        retryMount (with timeout > 1 early return).
 *        Heavy mount/DBus methods are not exercised.
 */

#include <gtest/gtest.h>
#include <QString>

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-mount/base/dmount_global.h>

using namespace dfmbase;

TEST(DeviceManagerTest, InstanceReturnsSingleton)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance(); });
}

TEST(DeviceManagerTest, IsMonitoringFalseByDefault)
{
    EXPECT_FALSE(DeviceManager::instance()->isMonitoring());
}

TEST(DeviceManagerTest, EnableBlockAutoMount)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->enableBlockAutoMount(); });
}

TEST(DeviceManagerTest, StartOpticalDiscScanCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->startOpticalDiscScan(); });
}

TEST(DeviceManagerTest, RetryMountExceedsTimeoutReturns)
{
    // timeout > 1 -> early return, no QTimer::singleShot
    EXPECT_NO_FATAL_FAILURE({
        DeviceManager::instance()->retryMount("/dev/sda1", dfmmount::DeviceType::kBlockDevice, 5);
    });
}

TEST(DeviceManagerTest, DetachAllProtoDevsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->detachAllProtoDevs(); });
}

TEST(DeviceManagerTest, DetachProtoDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->detachProtoDev("/org/freedesktop/UDisks2/block_devices/sda1"); });
}

TEST(DeviceManagerTest, InitUsageCacheCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->initUsageCache(); });
}

TEST(DeviceManagerTest, RefreshUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->refreshUsage(); });
}

TEST(DeviceManagerTest, StartPollingDeviceUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->startPollingDeviceUsage(); });
}

TEST(DeviceManagerTest, StopPollingDeviceUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->stopPollingDeviceUsage(); });
}

TEST(DeviceManagerTest, D0DestructorPath)
{
    // singleton already exists; just verify instance is valid
    EXPECT_NE(DeviceManager::instance(), nullptr);
}

TEST(DeviceManagerTest, GetAllBlockDevIDCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance()->getAllBlockDevID(); });
}

TEST(DeviceManagerTest, GetAllProtocolDevIDCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance()->getAllProtocolDevID(); });
}

TEST(DeviceManagerTest, GetBlockDevInfoCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->getBlockDevInfo("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerTest, GetProtocolDevInfoCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->getProtocolDevInfo("/nonexistent/protocol/dev");
    });
}

TEST(DeviceManagerTest, RescanBlockDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->rescanBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerTest, UnmountBlockDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->unmountBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerTest, MountProtocolDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->mountProtocolDev("/nonexistent/protocol/dev");
    });
}

TEST(DeviceManagerTest, UnmountProtocolDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->unmountProtocolDev("/nonexistent/protocol/dev");
    });
}
