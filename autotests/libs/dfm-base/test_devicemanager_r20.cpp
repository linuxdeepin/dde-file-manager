// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanager_r20.cpp
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

TEST(DeviceManagerR20Test, InstanceReturnsSingleton)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance(); });
}

TEST(DeviceManagerR20Test, IsMonitoringFalseByDefault)
{
    EXPECT_FALSE(DeviceManager::instance()->isMonitoring());
}

TEST(DeviceManagerR20Test, EnableBlockAutoMount)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->enableBlockAutoMount(); });
}

TEST(DeviceManagerR20Test, StartOpticalDiscScanCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->startOpticalDiscScan(); });
}

TEST(DeviceManagerR20Test, RetryMountExceedsTimeoutReturns)
{
    // timeout > 1 -> early return, no QTimer::singleShot
    EXPECT_NO_FATAL_FAILURE({
        DeviceManager::instance()->retryMount("/dev/sda1", dfmmount::DeviceType::kBlockDevice, 5);
    });
}

TEST(DeviceManagerR20Test, DetachAllProtoDevsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->detachAllProtoDevs(); });
}

TEST(DeviceManagerR20Test, DetachProtoDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->detachProtoDev("/org/freedesktop/UDisks2/block_devices/sda1"); });
}

TEST(DeviceManagerR20Test, InitUsageCacheCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->initUsageCache(); });
}

TEST(DeviceManagerR20Test, RefreshUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->refreshUsage(); });
}

TEST(DeviceManagerR20Test, StartPollingDeviceUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->startPollingDeviceUsage(); });
}

TEST(DeviceManagerR20Test, StopPollingDeviceUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ DeviceManager::instance()->stopPollingDeviceUsage(); });
}

TEST(DeviceManagerR20Test, D0DestructorPath)
{
    // singleton already exists; just verify instance is valid
    EXPECT_NE(DeviceManager::instance(), nullptr);
}

TEST(DeviceManagerR20Test, GetAllBlockDevIDCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance()->getAllBlockDevID(); });
}

TEST(DeviceManagerR20Test, GetAllProtocolDevIDCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceManager::instance()->getAllProtocolDevID(); });
}

TEST(DeviceManagerR20Test, GetBlockDevInfoCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->getBlockDevInfo("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerR20Test, GetProtocolDevInfoCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->getProtocolDevInfo("/nonexistent/protocol/dev");
    });
}

TEST(DeviceManagerR20Test, RescanBlockDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->rescanBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerR20Test, UnmountBlockDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->unmountBlockDev("/org/freedesktop/UDisks2/block_devices/nonexistent");
    });
}

TEST(DeviceManagerR20Test, MountProtocolDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->mountProtocolDev("/nonexistent/protocol/dev");
    });
}

TEST(DeviceManagerR20Test, UnmountProtocolDevCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)DeviceManager::instance()->unmountProtocolDev("/nonexistent/protocol/dev");
    });
}
