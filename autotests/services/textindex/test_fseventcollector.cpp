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
