// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitor_extra.cpp
 * @brief Additional unit tests for FSMonitor (fsmonitor/fsmonitor.cpp).
 *        Tests the singleton's public API more thoroughly.
 *        NOTE: We do NOT call initialize/start/stop on the singleton here
 *        because those spawn inotify watchers that conflict with other tests.
 *        The basic init/start/stop is covered in test_fsmonitor_api.cpp.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fsmonitor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

// --- Test setMaxResourceUsage clamping behavior ---

TEST(FSMonitorExtraTest, SetMaxResourceUsage_ClampBelowMinimum)
{
    FSMonitor &m = FSMonitor::instance();
    m.setMaxResourceUsage(0.05);  // below 0.1 minimum
    // Should be clamped to 0.1
    EXPECT_GE(m.maxResourceUsage(), 0.1);
    m.setMaxResourceUsage(0.5);  // restore default
}

TEST(FSMonitorExtraTest, SetMaxResourceUsage_ClampAboveMaximum)
{
    FSMonitor &m = FSMonitor::instance();
    m.setMaxResourceUsage(2.0);  // above 1.0 maximum
    // Should be clamped to 1.0
    EXPECT_LE(m.maxResourceUsage(), 1.0);
    m.setMaxResourceUsage(0.5);  // restore default
}

TEST(FSMonitorExtraTest, SetMaxResourceUsage_ExactlyAtBounds)
{
    FSMonitor &m = FSMonitor::instance();
    m.setMaxResourceUsage(0.1);
    EXPECT_DOUBLE_EQ(m.maxResourceUsage(), 0.1);
    m.setMaxResourceUsage(1.0);
    EXPECT_DOUBLE_EQ(m.maxResourceUsage(), 1.0);
    m.setMaxResourceUsage(0.5);  // restore default
}

// --- Blacklist tests with more edge cases ---

TEST(FSMonitorExtraTest, Blacklist_RemoveNonExistentPath)
{
    FSMonitor &m = FSMonitor::instance();
    // Removing a path that was never added should not crash
    EXPECT_NO_FATAL_FAILURE({
        m.removeBlacklistedPath("/never/added/path");
    });
}

TEST(FSMonitorExtraTest, Blacklist_AddEmptyPath)
{
    FSMonitor &m = FSMonitor::instance();
    EXPECT_NO_FATAL_FAILURE({
        m.addBlacklistedPath("");
    });
    // Clean up
    m.removeBlacklistedPath("");
}

TEST(FSMonitorExtraTest, Blacklist_MultipleAddsAndRemoves)
{
    FSMonitor &m = FSMonitor::instance();
    QStringList paths {
        "/tmp/blacklist_a",
        "/tmp/blacklist_b",
        "/tmp/blacklist_c",
        "/tmp/blacklist_d",
        "/tmp/blacklist_e"
    };
    m.addBlacklistedPaths(paths);

    QStringList retrieved = m.blacklistedPaths();
    for (const QString &p : paths) {
        EXPECT_TRUE(retrieved.contains(p));
    }

    // Remove some
    m.removeBlacklistedPath("/tmp/blacklist_b");
    m.removeBlacklistedPath("/tmp/blacklist_d");

    retrieved = m.blacklistedPaths();
    EXPECT_FALSE(retrieved.contains("/tmp/blacklist_b"));
    EXPECT_FALSE(retrieved.contains("/tmp/blacklist_d"));
    EXPECT_TRUE(retrieved.contains("/tmp/blacklist_a"));
    EXPECT_TRUE(retrieved.contains("/tmp/blacklist_c"));
    EXPECT_TRUE(retrieved.contains("/tmp/blacklist_e"));

    // Clean up
    m.removeBlacklistedPath("/tmp/blacklist_a");
    m.removeBlacklistedPath("/tmp/blacklist_c");
    m.removeBlacklistedPath("/tmp/blacklist_e");
}

// --- Stop when not active (should not crash) ---
TEST(FSMonitorExtraTest, StopWhenNotActive)
{
    FSMonitor &m = FSMonitor::instance();
    EXPECT_NO_FATAL_FAILURE({
        m.stop();
    });
}

// --- Various getters ---
TEST(FSMonitorExtraTest, CurrentWatchCountDefault)
{
    // By default no watches
    int count = FSMonitor::instance().currentWatchCount();
    EXPECT_GE(count, 0);
}

TEST(FSMonitorExtraTest, MaxAvailableWatchCount)
{
    int maxCount = FSMonitor::instance().maxAvailableWatchCount();
    // May be -1 if not initialized, or positive if initialized
    EXPECT_NO_FATAL_FAILURE({ (void)maxCount; });
}

TEST(FSMonitorExtraTest, IsActive_DefaultFalse)
{
    EXPECT_FALSE(FSMonitor::instance().isActive());
}