// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indextask.cpp
 * @brief Unit tests for IndexTask getters/setters (indextask.cpp)
 *
 * start() asserts it runs off the main thread, so we only exercise the
 * non-threaded API surface here.
 */

#include <gtest/gtest.h>
#include <QString>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/indextask.h"
#include "services/textindex/task/taskhandler.h"
#include "services/textindex/utils/taskstate.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

static TaskHandler makeNoopHandler()
{
    return [](const QString &path, TaskState &state) -> HandlerResult {
        HandlerResult r;
        r.success = true;
        return r;
    };
}

TEST(IndexTaskTest, ConstructAndAccessors)
{
    IndexTask task(IndexTask::Type::Create, "/tmp/dfm_test_path", makeNoopHandler());
    EXPECT_EQ(task.taskPath(), QString("/tmp/dfm_test_path"));
    EXPECT_EQ(task.taskType(), IndexTask::Type::Create);
    EXPECT_EQ(task.status(), IndexTask::Status::NotStarted);
    EXPECT_FALSE(task.isRunning());
}

TEST(IndexTaskTest, SilentAccessors)
{
    IndexTask task(IndexTask::Type::Update, "/tmp/p", makeNoopHandler());
    EXPECT_FALSE(task.silent());
    task.setSilent(true);
    EXPECT_TRUE(task.silent());
    task.setSilent(false);
    EXPECT_FALSE(task.silent());
}

TEST(IndexTaskTest, IndexCorruptedAccessors)
{
    IndexTask task(IndexTask::Type::CreateFileList, "/tmp/p", makeNoopHandler());
    EXPECT_FALSE(task.isIndexCorrupted());
    task.setIndexCorrupted(true);
    EXPECT_TRUE(task.isIndexCorrupted());
    task.setIndexCorrupted(false);
    EXPECT_FALSE(task.isIndexCorrupted());
}

TEST(IndexTaskTest, StopWhenNotRunningNoCrash)
{
    IndexTask task(IndexTask::Type::RemoveFileList, "/tmp/p", makeNoopHandler());
    EXPECT_NO_FATAL_FAILURE({ task.stop(); });
}

TEST(TaskStateTest, DefaultNotRunning)
{
    TaskState s;
    EXPECT_FALSE(s.isRunning());
}

TEST(TaskStateTest, StartStop)
{
    TaskState s;
    s.start();
    EXPECT_TRUE(s.isRunning());
    s.stop();
    EXPECT_FALSE(s.isRunning());
}

TEST(TaskStateTest, Silent)
{
    TaskState s;
    EXPECT_FALSE(s.isSilent());
    s.setSilent(true);
    EXPECT_TRUE(s.isSilent());
}
