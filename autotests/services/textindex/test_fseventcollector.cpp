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

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fseventcollector.h"

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
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ (void)collector.start(); });
}
