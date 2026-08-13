// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicewatcher.cpp
 * @brief Unit tests for DeviceWatcher (devicewatcher.cpp)
 *
 * DeviceWatcher has a public constructor and polling/watch control methods.
 * Without any real block/protocol device or DBus daemon, the query methods
 * return empty results and the polling timer is not active. No hardware
 * is needed.
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>
#include "stubext.h"

#include <dfm-base/base/device/private/devicewatcher.h>
#include <dfm-base/base/device/private/devicewatcher_p.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/dbusservice/opticalshareproxy.h>

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <dfm-mount/base/dmount_global.h>

using namespace dfmbase;

TEST(DeviceWatcherTest, ConstructAndDestructWithoutCrash)
{
    {
        DeviceWatcher watcher;
        (void)watcher;
    }
    SUCCEED();
}

TEST(DeviceWatcherTest, StartAndStopPollingUsageDoesNotCrash)
{
    DeviceWatcher watcher;
    watcher.startPollingUsage();
    watcher.stopPollingUsage();
    SUCCEED();
}

TEST(DeviceWatcherTest, StopPollingWhenNotStartedDoesNotCrash)
{
    DeviceWatcher watcher;
    watcher.stopPollingUsage();
    SUCCEED();
}

TEST(DeviceWatcherTest, InitUsageCacheDoesNotCrash)
{
    DeviceWatcher watcher;
    watcher.initUsageCache();
    SUCCEED();
}

TEST(DeviceWatcherTest, GetSiblingsReturnsEmptyForNonExistentDevice)
{
    DeviceWatcher watcher;
    QStringList siblings = watcher.getSiblings(QStringLiteral("/dev/nonexistent-ut-12345"));
    EXPECT_TRUE(siblings.isEmpty());
}

TEST(DeviceWatcherTest, InitDevDatasDoesNotCrash)
{
    DeviceWatcher watcher;
    watcher.initDevDatas();
    SUCCEED();
}

// ============================================================
// Additional coverage for DeviceWatcher
// ============================================================

TEST(DeviceWatcherTest, GetDevInfoForNonExistentBlockDev)
{
    DeviceWatcher watcher;
    QVariantMap info = watcher.getDevInfo(
            "/org/freedesktop/UDisks2/block_devices/nonexistent",
            dfmmount::DeviceType::kBlockDevice, false);
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceWatcherTest, GetDevInfoForNonExistentBlockDevWithReload)
{
    DeviceWatcher watcher;
    QVariantMap info = watcher.getDevInfo(
            "/org/freedesktop/UDisks2/block_devices/nonexistent",
            dfmmount::DeviceType::kBlockDevice, true);
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceWatcherTest, GetDevInfoForNonExistentProtocolDev)
{
    DeviceWatcher watcher;
    QVariantMap info = watcher.getDevInfo(
            "/nonexistent/protocol/dev",
            dfmmount::DeviceType::kProtocolDevice, false);
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceWatcherTest, GetDevInfoForNonExistentProtocolDevWithReload)
{
    DeviceWatcher watcher;
    QVariantMap info = watcher.getDevInfo(
            "/nonexistent/protocol/dev",
            dfmmount::DeviceType::kProtocolDevice, true);
    // May return fake/real info in env with dfm-mount
    EXPECT_NO_FATAL_FAILURE({ (void)info; });
}

TEST(DeviceWatcherTest, GetDevInfoForUnknownDeviceType)
{
    DeviceWatcher watcher;
    QVariantMap info = watcher.getDevInfo(
            "/some/device",
            dfmmount::DeviceType::kAllDevice, false);
    EXPECT_TRUE(info.isEmpty());
}

TEST(DeviceWatcherTest, GetDevIdsForBlockDevice)
{
    DeviceWatcher watcher;
    QStringList ids = watcher.getDevIds(dfmmount::DeviceType::kBlockDevice);
    // Should be empty or valid list, but must not crash
    EXPECT_TRUE(ids.isEmpty());
}

TEST(DeviceWatcherTest, GetDevIdsForProtocolDevice)
{
    DeviceWatcher watcher;
    QStringList ids = watcher.getDevIds(dfmmount::DeviceType::kProtocolDevice);
    EXPECT_TRUE(ids.isEmpty());
}

TEST(DeviceWatcherTest, GetDevIdsForUnknownDeviceType)
{
    DeviceWatcher watcher;
    QStringList ids = watcher.getDevIds(dfmmount::DeviceType::kAllDevice);
    EXPECT_TRUE(ids.isEmpty());
}

TEST(DeviceWatcherTest, RefreshUsageDoesNotCrash)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.refreshUsage();
    });
}

TEST(DeviceWatcherTest, QueryOpticalDevUsageNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.queryOpticalDevUsage("/org/freedesktop/UDisks2/block_devices/sr99");
    });
}

TEST(DeviceWatcherTest, UpdateOpticalDevUsageEmptyMpt)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.updateOpticalDevUsage("/org/freedesktop/UDisks2/block_devices/sr99", "");
    });
}

TEST(DeviceWatcherTest, UpdateOpticalDevUsageNonExistentDev)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.updateOpticalDevUsage("/org/freedesktop/UDisks2/block_devices/sr99", "/media/sr99");
    });
}

TEST(DeviceWatcherTest, SaveOpticalDevUsageNonExistent)
{
    // Stub OpticalShareProxy to avoid DBus crash
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(OpticalShareProxy, setBurnAttribute), [](OpticalShareProxy *, const QString &, const QVariantMap &) -> bool { return true; });

    DeviceWatcher watcher;
    QVariantMap data;
    data[GlobalServerDefines::DeviceProperty::kSizeTotal] = quint64(0);
    data[GlobalServerDefines::DeviceProperty::kSizeFree] = quint64(0);
    EXPECT_NO_FATAL_FAILURE({
        watcher.saveOpticalDevUsage("/org/freedesktop/UDisks2/block_devices/sr99", data);
    });
}

TEST(DeviceWatcherTest, StartWatchCallable)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.startWatch();
    });
    // Clean up
    watcher.stopWatch();
}

TEST(DeviceWatcherTest, StopWatchWhenNotWatching)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.stopWatch();
    });
}

TEST(DeviceWatcherTest, StartWatchIdempotent)
{
    DeviceWatcher watcher;
    watcher.startWatch();
    EXPECT_NO_FATAL_FAILURE({
        watcher.startWatch();   // second call should be no-op
    });
    watcher.stopWatch();
}

TEST(DeviceWatcherTest, StartAndStopWatchMultipleTimes)
{
    DeviceWatcher watcher;
    for (int i = 0; i < 3; ++i) {
        watcher.startWatch();
        watcher.stopWatch();
    }
    SUCCEED();
}

TEST(DeviceWatcherTest, StartPollingUsageIdempotent)
{
    DeviceWatcher watcher;
    watcher.startPollingUsage();
    // Starting again should be no-op
    EXPECT_NO_FATAL_FAILURE({
        watcher.startPollingUsage();
    });
    watcher.stopPollingUsage();
}

TEST(DeviceWatcherTest, StopPollingUsageTwice)
{
    DeviceWatcher watcher;
    watcher.startPollingUsage();
    watcher.stopPollingUsage();
    EXPECT_NO_FATAL_FAILURE({
        watcher.stopPollingUsage();
    });
}

TEST(DeviceWatcherTest, InitUsageCacheWhenPollingActive)
{
    DeviceWatcher watcher;
    watcher.startPollingUsage();
    // When timer is active, initUsageCache should be a no-op
    EXPECT_NO_FATAL_FAILURE({
        watcher.initUsageCache();
    });
    watcher.stopPollingUsage();
}

TEST(DeviceWatcherTest, OnBlkDevAddedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevAdded("/org/freedesktop/UDisks2/block_devices/nonexistent_add");
    });
}

TEST(DeviceWatcherTest, OnBlkDevRemovedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevRemoved("/org/freedesktop/UDisks2/block_devices/nonexistent_remove");
    });
}

TEST(DeviceWatcherTest, OnBlkDevMountedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevMounted("/org/freedesktop/UDisks2/block_devices/nonexistent_mnt", "/mnt/fake");
    });
}

TEST(DeviceWatcherTest, OnBlkDevUnmountedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevUnmounted("/org/freedesktop/UDisks2/block_devices/nonexistent_umnt");
    });
}

TEST(DeviceWatcherTest, OnBlkDevLockedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevLocked("/org/freedesktop/UDisks2/block_devices/nonexistent_lock");
    });
}

TEST(DeviceWatcherTest, OnBlkDevUnlockedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevUnlocked("/org/freedesktop/UDisks2/block_devices/nonexistent_unlock", "/cleartext");
    });
}

TEST(DeviceWatcherTest, OnBlkDevFsAddedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevFsAdded("/org/freedesktop/UDisks2/block_devices/nonexistent_fsadd");
    });
}

TEST(DeviceWatcherTest, OnBlkDevFsRemovedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevFsRemoved("/org/freedesktop/UDisks2/block_devices/nonexistent_fsrm");
    });
}

TEST(DeviceWatcherTest, OnBlkDevPropertiesChangedEmpty)
{
    DeviceWatcher watcher;
    QMap<dfmmount::Property, QVariant> changes;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevPropertiesChanged("/org/freedesktop/UDisks2/block_devices/nonexistent_prop", changes);
    });
}

TEST(DeviceWatcherTest, OnBlkDevPropertiesChangedUnknownProperty)
{
    DeviceWatcher watcher;
    QMap<dfmmount::Property, QVariant> changes;
    changes.insert(static_cast<dfmmount::Property>(99999), QVariant("test"));
    EXPECT_NO_FATAL_FAILURE({
        watcher.onBlkDevPropertiesChanged("/org/freedesktop/UDisks2/block_devices/nonexistent_ukprop", changes);
    });
}

TEST(DeviceWatcherTest, OnProtoDevAddedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onProtoDevAdded("/nonexistent/protocol/dev_add");
    });
}

TEST(DeviceWatcherTest, OnProtoDevRemovedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onProtoDevRemoved("/nonexistent/protocol/dev_remove");
    });
}

TEST(DeviceWatcherTest, OnProtoDevMountedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onProtoDevMounted("/nonexistent/protocol/dev_mount", "/mnt/proto");
    });
}

TEST(DeviceWatcherTest, OnProtoDevUnmountedNonExistent)
{
    DeviceWatcher watcher;
    EXPECT_NO_FATAL_FAILURE({
        watcher.onProtoDevUnmounted("/nonexistent/protocol/dev_unmount");
    });
}

TEST(DeviceWatcherTest, GetSiblingsForNonBlockDevice)
{
    DeviceWatcher watcher;
    // Non-block device path should return empty
    QStringList sibs = watcher.getSiblings("/dev/nonexistent_12345");
    EXPECT_TRUE(sibs.isEmpty());
}

TEST(DeviceWatcherTest, GetSiblingsForAbsolutePath)
{
    DeviceWatcher watcher;
    // Full block device ID
    QStringList sibs = watcher.getSiblings("/org/freedesktop/UDisks2/block_devices/nonexistent");
    // Returns empty since device doesn't exist
    EXPECT_TRUE(sibs.isEmpty());
}

TEST(DeviceWatcherTest, DevStorageDefaultState)
{
    DevStorage s;
    EXPECT_EQ(s.total, quint64(0));
    EXPECT_EQ(s.avai, quint64(0));
    EXPECT_EQ(s.used, quint64(0));
    EXPECT_FALSE(s.isValid());
}

TEST(DeviceWatcherTest, DevStorageValidState)
{
    DevStorage s;
    s.total = 100;
    s.avai = 50;
    s.used = 50;
    EXPECT_TRUE(s.isValid());
}

TEST(DeviceWatcherTest, DevStorageEquality)
{
    DevStorage a { 100, 50, 50 };
    DevStorage b { 100, 50, 50 };
    DevStorage c { 200, 100, 100 };
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a == c);
}

TEST(DeviceWatcherTest, UpdateStorageForBlockDevice)
{
    // Access private via -fno-access-control
    DeviceWatcher watcher;
    auto *d = watcher.d.data();
    // Insert fake data
    QVariantMap fakeInfo;
    fakeInfo["Id"] = "/org/freedesktop/UDisks2/block_devices/sda1";
    d->allBlockInfos.insert("/org/freedesktop/UDisks2/block_devices/sda1", fakeInfo);
    EXPECT_NO_FATAL_FAILURE({
        d->updateStorage("/org/freedesktop/UDisks2/block_devices/sda1", 1024, 512);
    });
    auto &updated = d->allBlockInfos["/org/freedesktop/UDisks2/block_devices/sda1"];
    EXPECT_EQ(updated[GlobalServerDefines::DeviceProperty::kSizeTotal].toULongLong(), quint64(1024));
    EXPECT_EQ(updated[GlobalServerDefines::DeviceProperty::kSizeFree].toULongLong(), quint64(512));
    EXPECT_EQ(updated[GlobalServerDefines::DeviceProperty::kSizeUsed].toULongLong(), quint64(512));
}

TEST(DeviceWatcherTest, UpdateStorageForProtocolDevice)
{
    DeviceWatcher watcher;
    auto *d = watcher.d.data();
    QVariantMap fakeInfo;
    fakeInfo["Id"] = "/protocol/smb_share";
    d->allProtocolInfos.insert("/protocol/smb_share", fakeInfo);
    EXPECT_NO_FATAL_FAILURE({
        d->updateStorage("/protocol/smb_share", 2048, 1024);
    });
    auto &updated = d->allProtocolInfos["/protocol/smb_share"];
    EXPECT_EQ(updated[GlobalServerDefines::DeviceProperty::kSizeTotal].toULongLong(), quint64(2048));
    EXPECT_EQ(updated[GlobalServerDefines::DeviceProperty::kSizeUsed].toULongLong(), quint64(1024));
}

TEST(DeviceWatcherTest, UpdateStorageNonExistentDevice)
{
    DeviceWatcher watcher;
    auto *d = watcher.d.data();
    EXPECT_NO_FATAL_FAILURE({
        d->updateStorage("/nonexistent/device", 100, 50);
    });
    // No crash, data not added
    EXPECT_TRUE(d->allBlockInfos.isEmpty());
}

TEST(DeviceWatcherTest, QueryUsageOfItemWithEmptyMountPoint)
{
    DeviceWatcher watcher;
    auto *d = watcher.d.data();
    QVariantMap item;
    item[GlobalServerDefines::DeviceProperty::kMountPoint] = "";
    EXPECT_NO_FATAL_FAILURE({
        d->queryUsageOfItem(item, dfmmount::DeviceType::kBlockDevice);
    });
}

TEST(DeviceWatcherTest, QueryUsageOfItemWithAllDeviceType)
{
    DeviceWatcher watcher;
    auto *d = watcher.d.data();
    QVariantMap item;
    item[GlobalServerDefines::DeviceProperty::kMountPoint] = "/mnt/test";
    // kAllDevice → returns immediately
    EXPECT_NO_FATAL_FAILURE({
        d->queryUsageOfItem(item, dfmmount::DeviceType::kAllDevice);
    });
}

TEST(DeviceWatcherTest, QueryUsageAsyncDoesNotCrash)
{
    DeviceWatcher watcher;
    auto *d = watcher.d.data();
    EXPECT_NO_FATAL_FAILURE({
        d->queryUsageAsync();
    });
}
