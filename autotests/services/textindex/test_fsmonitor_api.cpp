// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitor_api.cpp
 * @brief Unit tests for FSMonitor public API (fsmonitor/fsmonitor.cpp) — the
 *        dependency-light subset: instance, isActive (false by default),
 *        blacklist management (addBlacklistedPath/addBlacklistedPaths/
 *        removeBlacklistedPath/blacklistedPaths), resource usage getters
 *        (maxResourceUsage/currentWatchCount/maxAvailableWatchCount), and
 *        setUseFastScan/useFastScan. initialize/start/stop are NOT invoked
 *        (they spawn inotify watchers and threads).
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fsmonitor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(FSMonitorApiTest, InstanceReturnsSingleton)
{
    FSMonitor &a = FSMonitor::instance();
    FSMonitor &b = FSMonitor::instance();
    EXPECT_EQ(&a, &b);
}

TEST(FSMonitorApiTest, IsActiveFalseByDefault)
{
    EXPECT_FALSE(FSMonitor::instance().isActive());
}

TEST(FSMonitorApiTest, AddBlacklistedPathAndRetrieve)
{
    FSMonitor &m = FSMonitor::instance();
    m.addBlacklistedPath("/tmp/dfm_test_blacklist_1");
    QStringList paths = m.blacklistedPaths();
    EXPECT_TRUE(paths.contains("/tmp/dfm_test_blacklist_1"));
    m.removeBlacklistedPath("/tmp/dfm_test_blacklist_1");
}

TEST(FSMonitorApiTest, AddBlacklistedPathsBatch)
{
    FSMonitor &m = FSMonitor::instance();
    QStringList batch { "/tmp/dfm_test_bl_a", "/tmp/dfm_test_bl_b" };
    m.addBlacklistedPaths(batch);
    QStringList paths = m.blacklistedPaths();
    EXPECT_TRUE(paths.contains("/tmp/dfm_test_bl_a"));
    EXPECT_TRUE(paths.contains("/tmp/dfm_test_bl_b"));
    m.removeBlacklistedPath("/tmp/dfm_test_bl_a");
    m.removeBlacklistedPath("/tmp/dfm_test_bl_b");
}

TEST(FSMonitorApiTest, RemoveBlacklistedPath)
{
    FSMonitor &m = FSMonitor::instance();
    m.addBlacklistedPath("/tmp/dfm_test_bl_remove");
    m.removeBlacklistedPath("/tmp/dfm_test_bl_remove");
    EXPECT_FALSE(m.blacklistedPaths().contains("/tmp/dfm_test_bl_remove"));
}

TEST(FSMonitorApiTest, MaxResourceUsageReturnsDouble)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FSMonitor::instance().maxResourceUsage(); });
}

TEST(FSMonitorApiTest, CurrentWatchCountZeroByDefault)
{
    EXPECT_EQ(FSMonitor::instance().currentWatchCount(), 0);
}

TEST(FSMonitorApiTest, MaxAvailableWatchCountReturnsInt)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FSMonitor::instance().maxAvailableWatchCount(); });
}

TEST(FSMonitorApiTest, SetUseFastScanWhenInactive)
{
    FSMonitor &m = FSMonitor::instance();
    m.setUseFastScan(true);
    EXPECT_TRUE(m.useFastScan());
    m.setUseFastScan(false);
    EXPECT_FALSE(m.useFastScan());
}

TEST(FSMonitorApiTest, SetMaxResourceUsageCallable)
{
    EXPECT_NO_FATAL_FAILURE({ FSMonitor::instance().setMaxResourceUsage(50.0); });
}
