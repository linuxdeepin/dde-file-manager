// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fseventcollector.cpp
 * @brief Unit tests for FSEventCollector public API (fseventcollector.cpp)
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QHash>

#include "stubext.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fseventcollector.h"
#include "services/textindex/fsmonitor/fsmonitor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

static FSEventCollector::PathPredicate alwaysTrue()
{
    return [](const QString &) -> bool { return true; };
}

TEST(FSEventCollectorTest, ConstructAndDestruct)
{
    FSEventCollector collector(alwaysTrue());
    SUCCEED();
}

TEST(FSEventCollectorTest, IsActiveDefaultFalse)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_FALSE(collector.isActive());
}

TEST(FSEventCollectorTest, InitializeWithEmptyRootPaths)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ (void)collector.initialize({}); });
}

TEST(FSEventCollectorTest, SetCollectionInterval)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.setCollectionInterval(10); });
}

TEST(FSEventCollectorTest, SetMaxEventCount)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.setMaxEventCount(1000); });
}

TEST(FSEventCollectorTest, ClearEventsNoCrash)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.clearEvents(); });
}

TEST(FSEventCollectorTest, FlushEventsWhenInactive)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.flushEvents(); });
}

TEST(FSEventCollectorTest, StopWhenInactive)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.stop(); });
}

TEST(FSEventCollectorTest, CreatedFilesEmpty)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_TRUE(collector.createdFiles().isEmpty());
}

TEST(FSEventCollectorTest, DeletedFilesEmpty)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_TRUE(collector.deletedFiles().isEmpty());
}

TEST(FSEventCollectorTest, ModifiedFilesEmpty)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_TRUE(collector.modifiedFiles().isEmpty());
}

TEST(FSEventCollectorTest, MovedFilesEmpty)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_TRUE(collector.movedFiles().isEmpty());
}

TEST(FSEventCollectorTest, StartWithoutInitMayFailGracefully)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(FSMonitor, start), [](FSMonitor *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ (void)collector.start(); });
}

// ---- Round 10: cover getter methods ----

TEST(FSEventCollectorTest, CollectionIntervalDefault)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ (void)collector.collectionInterval(); });
}

TEST(FSEventCollectorTest, CollectionIntervalAfterSet)
{
    FSEventCollector collector(alwaysTrue());
    collector.setCollectionInterval(42);
    EXPECT_EQ(collector.collectionInterval(), 42);
}

TEST(FSEventCollectorTest, MaxEventCountDefault)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ (void)collector.maxEventCount(); });
}

TEST(FSEventCollectorTest, MaxEventCountAfterSet)
{
    FSEventCollector collector(alwaysTrue());
    collector.setMaxEventCount(5000);
    EXPECT_EQ(collector.maxEventCount(), 5000);
}

TEST(FSEventCollectorTest, CreatedFilesCountZero)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_EQ(collector.createdFilesCount(), 0);
}

TEST(FSEventCollectorTest, DeletedFilesCountZero)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_EQ(collector.deletedFilesCount(), 0);
}

TEST(FSEventCollectorTest, ModifiedFilesCountZero)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_EQ(collector.modifiedFilesCount(), 0);
}

TEST(FSEventCollectorTest, MovedFilesCountZero)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_EQ(collector.movedFilesCount(), 0);
}

TEST(FSEventCollectorTest, TotalEventsCountZero)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_EQ(collector.totalEventsCount(), 0);
}

TEST(FSEventCollectorTest, ConstructWithExplicitFSMonitor)
{
    FSMonitor &monitor = FSMonitor::instance();
    FSEventCollector collector(alwaysTrue(), monitor);
    EXPECT_FALSE(collector.isActive());
    EXPECT_NO_FATAL_FAILURE({ (void)collector.collectionInterval(); });
}

// ---- Additional coverage for FSEventCollector ----

TEST(FSEventCollectorTest, SetCollectionInterval_InvalidValues)
{
    FSEventCollector collector(alwaysTrue());
    collector.setCollectionInterval(-1);
    collector.setCollectionInterval(0);
    // Both should be ignored (warning logged), default 180s unchanged
    EXPECT_EQ(collector.collectionInterval(), 180);
}

TEST(FSEventCollectorTest, SetMaxEventCount_InvalidValues)
{
    FSEventCollector collector(alwaysTrue());
    collector.setMaxEventCount(-10);
    collector.setMaxEventCount(0);
    // Both ignored, default 10000 unchanged
    EXPECT_EQ(collector.maxEventCount(), 10000);
}

TEST(FSEventCollectorTest, SetCollectionInterval_WhileActive_RestartsTimer)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(FSMonitor, start), [](FSMonitor *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(ADDR(FSMonitor, initialize), [](FSMonitor *, const QStringList &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    FSEventCollector collector(alwaysTrue());
    ASSERT_TRUE(collector.initialize({ "/tmp" }));
    ASSERT_TRUE(collector.start());
    EXPECT_TRUE(collector.isActive());

    // Changing interval while active should restart timer
    collector.setCollectionInterval(5);
    EXPECT_EQ(collector.collectionInterval(), 5);

    collector.stop();
    EXPECT_FALSE(collector.isActive());
}

TEST(FSEventCollectorTest, Initialize_NonExistentPaths_ReturnsFalse)
{
    FSEventCollector collector(alwaysTrue());
    bool result = collector.initialize({ "/nonexistent/path/abc123" });
    EXPECT_FALSE(result);
}

TEST(FSEventCollectorTest, Initialize_MixedValidInvalidPaths)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(FSMonitor, initialize), [](FSMonitor *, const QStringList &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    FSEventCollector collector(alwaysTrue());
    // /tmp exists, /nonexistent does not
    bool result = collector.initialize({ "/tmp", "/nonexistent/path" });
    // Should succeed because at least one path is valid
    EXPECT_TRUE(result);
}

TEST(FSEventCollectorTest, StartAlreadyActive_ReturnsTrue)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(FSMonitor, start), [](FSMonitor *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(ADDR(FSMonitor, initialize), [](FSMonitor *, const QStringList &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    FSEventCollector collector(alwaysTrue());
    ASSERT_TRUE(collector.initialize({ "/tmp" }));
    ASSERT_TRUE(collector.start());
    // Calling start again when already active returns true (no-op)
    EXPECT_TRUE(collector.start());

    collector.stop();
}

TEST(FSEventCollectorTest, Start_WhenMonitorFails_ReturnsFalse)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(FSMonitor, initialize), [](FSMonitor *, const QStringList &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(ADDR(FSMonitor, start), [](FSMonitor *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    FSEventCollector collector(alwaysTrue());
    ASSERT_TRUE(collector.initialize({ "/tmp" }));
    EXPECT_FALSE(collector.start());
}

TEST(FSEventCollectorTest, StopWhileNotActive_NoCrash)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.stop(); });
    EXPECT_FALSE(collector.isActive());
}

TEST(FSEventCollectorTest, ClearEvents_ResetsAllCounts)
{
    FSEventCollector collector(alwaysTrue());
    // Events should be empty by default, but verify clearEvents is safe
    EXPECT_NO_FATAL_FAILURE({ collector.clearEvents(); });
    EXPECT_EQ(collector.totalEventsCount(), 0);
    EXPECT_EQ(collector.createdFilesCount(), 0);
    EXPECT_EQ(collector.deletedFilesCount(), 0);
    EXPECT_EQ(collector.modifiedFilesCount(), 0);
    EXPECT_EQ(collector.movedFilesCount(), 0);
}

TEST(FSEventCollectorTest, FlushEvents_ClearsAllCounts)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.flushEvents(); });
    EXPECT_EQ(collector.totalEventsCount(), 0);
}

TEST(FSEventCollectorTest, CollectionInterval_DefaultIs180)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_EQ(collector.collectionInterval(), 180);
}

TEST(FSEventCollectorTest, MaxEventCount_DefaultIs10000)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_EQ(collector.maxEventCount(), 10000);
}
