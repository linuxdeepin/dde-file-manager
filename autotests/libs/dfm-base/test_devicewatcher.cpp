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
#include <dfm-base/base/device/private/devicewatcher.h>

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
