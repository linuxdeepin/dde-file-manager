// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskqueueutils.cpp
 * @brief Unit tests for TaskQueueUtils (task/taskqueueutils.cpp) — pure path
 *        rewrite logic: buildDirectoryMovePlans and rewriteQueuedTasksForDirectory
 *        Move (empty-queue early return + real rewrite). Also exercises
 *        PathCalculator helpers indirectly.
 */

#include <gtest/gtest.h>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QQueue>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/taskqueueutils.h"
#include "services/textindex/task/taskmanager.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(TaskQueueUtilsTest, BuildDirectoryMovePlansEmptyReturnsEmpty)
{
    QHash<QString, QString> empty;
    auto plans = TaskQueueUtils::buildDirectoryMovePlans(empty);
    EXPECT_TRUE(plans.isEmpty());
}

TEST(TaskQueueUtilsTest, BuildDirectoryMovePlansNonDirExcluded)
{
    QHash<QString, QString> moves { { "/old/file.txt", "/new/file.txt" } };
    auto plans = TaskQueueUtils::buildDirectoryMovePlans(moves);
    EXPECT_TRUE(plans.isEmpty());   // file move, not directory move
}

TEST(TaskQueueUtilsTest, BuildDirectoryMovePlansDirectoryIncluded)
{
    QHash<QString, QString> moves { { "/old/dir/", "/new/dir/" } };
    auto plans = TaskQueueUtils::buildDirectoryMovePlans(moves);
    EXPECT_EQ(plans.size(), 1);
    EXPECT_EQ(plans[0].fromPath, QString("/old/dir/"));
    EXPECT_EQ(plans[0].toPath, QString("/new/dir/"));
}

TEST(TaskQueueUtilsTest, RewriteQueuedTasksEmptyQueueReturnsFalse)
{
    QQueue<TaskQueueItem> q;
    EXPECT_FALSE(TaskQueueUtils::rewriteQueuedTasksForDirectoryMove(q, "/old/", "/new/"));
}

TEST(TaskQueueUtilsTest, RewriteQueuedTasksEmptyFromReturnsFalse)
{
    QQueue<TaskQueueItem> q;
    TaskQueueItem item;
    item.path = "/old/dir/file.txt";
    q.enqueue(item);
    EXPECT_FALSE(TaskQueueUtils::rewriteQueuedTasksForDirectoryMove(q, QString(), "/new/"));
}

TEST(TaskQueueUtilsTest, RewriteQueuedTasksEmptyToReturnsFalse)
{
    QQueue<TaskQueueItem> q;
    TaskQueueItem item;
    item.path = "/old/dir/file.txt";
    q.enqueue(item);
    EXPECT_FALSE(TaskQueueUtils::rewriteQueuedTasksForDirectoryMove(q, "/old/", QString()));
}

TEST(TaskQueueUtilsTest, RewriteQueuedTasksRewritesPath)
{
    QQueue<TaskQueueItem> q;
    TaskQueueItem item;
    item.path = "/old/dir/file.txt";
    item.pathList = { "/old/dir/a.txt", "/other/b.txt" };
    item.fileList = { "/old/dir/c.txt" };
    q.enqueue(item);

    bool changed = TaskQueueUtils::rewriteQueuedTasksForDirectoryMove(q, "/old/dir/", "/new/dir/");
    EXPECT_TRUE(changed);
    EXPECT_EQ(q.head().path, QString("/new/dir//file.txt"));
    EXPECT_EQ(q.head().pathList.at(0), QString("/new/dir//a.txt"));
    EXPECT_EQ(q.head().pathList.at(1), QString("/other/b.txt"));   // not under /old/dir/
    EXPECT_EQ(q.head().fileList.at(0), QString("/new/dir//c.txt"));
}

TEST(TaskQueueUtilsTest, RewriteQueuedTasksNoMatchReturnsFalse)
{
    QQueue<TaskQueueItem> q;
    TaskQueueItem item;
    item.path = "/completely/different/path.txt";
    q.enqueue(item);
    bool changed = TaskQueueUtils::rewriteQueuedTasksForDirectoryMove(q, "/old/dir/", "/new/dir/");
    EXPECT_FALSE(changed);
}
