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
    EXPECT_NO_FATAL_FAILURE({ FSEventCollector collector(alwaysTrue()); });
}

TEST(FSEventCollectorTest, IsActiveDefaultFalse)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_FALSE(collector.isActive());
}

TEST(FSEventCollectorTest, InitializeWithEmptyRootPaths)
{
    FSEventCollector collector(alwaysTrue());
    EXPECT_NO_FATAL_FAILURE({ collector.initialize({}); });
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
    EXPECT_NO_FATAL_FAILURE({ collector.start(); });
}


TEST(FSEventCollectorTest, FSEventCollector)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ FSEventCollector obj; });
    }


TEST(FSEventCollectorTest, clearEvents)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE(obj.clearEvents());
}

TEST(FSEventCollectorTest, collectionInterval)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.collectionInterval(); });
}

TEST(FSEventCollectorTest, createdFiles)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.createdFiles(); });
}

TEST(FSEventCollectorTest, createdFilesCount)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.createdFilesCount(); });
}

TEST(FSEventCollectorTest, deletedFiles)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.deletedFiles(); });
}

TEST(FSEventCollectorTest, deletedFilesCount)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.deletedFilesCount(); });
}

TEST(FSEventCollectorTest, flushEvents)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE(obj.flushEvents());
}

TEST(FSEventCollectorTest, initialize)
{
    FSEventCollector obj;
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.initialize(_arg0); });
}

TEST(FSEventCollectorTest, isActive)
{
    FSEventCollector obj;
    bool result = obj.isActive();
    EXPECT_FALSE(result);
}

TEST(FSEventCollectorTest, maxEventCount)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.maxEventCount(); });
}

TEST(FSEventCollectorTest, modifiedFiles)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.modifiedFiles(); });
}

TEST(FSEventCollectorTest, modifiedFilesCount)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.modifiedFilesCount(); });
}

TEST(FSEventCollectorTest, movedFiles)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.movedFiles(); });
}

TEST(FSEventCollectorTest, movedFilesCount)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.movedFilesCount(); });
}

TEST(FSEventCollectorTest, setCollectionInterval)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE(obj.setCollectionInterval(0));
}

TEST(FSEventCollectorTest, setMaxEventCount)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE(obj.setMaxEventCount(0));
}

TEST(FSEventCollectorTest, start)
{
    FSEventCollector obj;
    bool result = obj.start();
    EXPECT_FALSE(result);
}

TEST(FSEventCollectorTest, stop)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE(obj.stop());
}

TEST(FSEventCollectorTest, totalEventsCount)
{
    FSEventCollector obj;
    EXPECT_NO_FATAL_FAILURE({ obj.totalEventsCount(); });
}
