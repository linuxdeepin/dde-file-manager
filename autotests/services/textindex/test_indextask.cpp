// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indextask.cpp
 * @brief Unit tests for IndexTask (task/indextask.cpp) — the dependency-light
 *        subset: ctor, dtor, and all accessors (taskPath/taskType/status/
 *        isIndexCorrupted/setIndexCorrupted/silent/setSilent) plus stop() and
 *        isRunning(). start() is intentionally NOT invoked (it runs the handler
 *        in a worker thread).
 */

#include <gtest/gtest.h>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/indextask.h"
#include "services/textindex/task/taskhandler.h"
#include "services/textindex/utils/taskstate.h"
#include <QSignalSpy>

using namespace SERVICETEXTINDEX_NAMESPACE;

static TaskHandler dummyHandler()
{
    return [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult {};
    };
}

TEST(IndexTaskTest, CtorSetsTypeAndPath)
{
    IndexTask t(IndexTask::Type::Create, "/tmp/test", dummyHandler());
    EXPECT_EQ(t.taskType(), IndexTask::Type::Create);
    EXPECT_EQ(t.taskPath(), QString("/tmp/test"));
}

TEST(IndexTaskTest, CtorAllTypes)
{
    IndexTask t1(IndexTask::Type::Update, "/a", dummyHandler());
    EXPECT_EQ(t1.taskType(), IndexTask::Type::Update);
    IndexTask t2(IndexTask::Type::CreateFileList, "/b", dummyHandler());
    EXPECT_EQ(t2.taskType(), IndexTask::Type::CreateFileList);
    IndexTask t3(IndexTask::Type::UpdateFileList, "/c", dummyHandler());
    EXPECT_EQ(t3.taskType(), IndexTask::Type::UpdateFileList);
    IndexTask t4(IndexTask::Type::RemoveFileList, "/d", dummyHandler());
    EXPECT_EQ(t4.taskType(), IndexTask::Type::RemoveFileList);
    IndexTask t5(IndexTask::Type::MoveFileList, "/e", dummyHandler());
    EXPECT_EQ(t5.taskType(), IndexTask::Type::MoveFileList);
}

TEST(IndexTaskTest, DefaultStatusNotStarted)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", dummyHandler());
    EXPECT_EQ(t.status(), IndexTask::Status::NotStarted);
    EXPECT_FALSE(t.isRunning());
}

TEST(IndexTaskTest, SilentRoundtrip)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", dummyHandler());
    EXPECT_FALSE(t.silent());
    t.setSilent(true);
    EXPECT_TRUE(t.silent());
    t.setSilent(false);
    EXPECT_FALSE(t.silent());
}

TEST(IndexTaskTest, IndexCorruptedRoundtrip)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", dummyHandler());
    EXPECT_FALSE(t.isIndexCorrupted());
    t.setIndexCorrupted(true);
    EXPECT_TRUE(t.isIndexCorrupted());
    t.setIndexCorrupted(false);
    EXPECT_FALSE(t.isIndexCorrupted());
}

TEST(IndexTaskTest, SetIndexCorruptedIdempotent)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", dummyHandler());
    t.setIndexCorrupted(true);
    t.setIndexCorrupted(true);   // already true -> no change
    EXPECT_TRUE(t.isIndexCorrupted());
}

TEST(IndexTaskTest, StopIsSafeBeforeStart)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", dummyHandler());
    EXPECT_NO_FATAL_FAILURE({ t.stop(); });
    EXPECT_FALSE(t.isRunning());
}

TEST(IndexTaskTest, DtorIsSafeWithoutStart)
{
    {
        IndexTask t(IndexTask::Type::Create, "/tmp/dtor_test", dummyHandler());
        EXPECT_EQ(t.taskPath(), QString("/tmp/dtor_test"));
    }
    SUCCEED();
}

// ---- TaskState inline class tests (restored from original test file) ----

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

static TaskHandler noopHandler()
{
    return [](const QString &, TaskState &) -> HandlerResult { return HandlerResult {}; };
}
TEST(IndexTaskTest, OnProgressChangedNoEmitWhenNotRunning)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    QSignalSpy spy(&t, &IndexTask::progressChanged);
    t.onProgressChanged(10, 100);   // private; not running -> no emit
    EXPECT_EQ(spy.count(), 0);
}
TEST(IndexTaskTest, ThrottleCpuUsageSkipsWhenNotSilent)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    EXPECT_FALSE(t.silent());
    EXPECT_NO_FATAL_FAILURE({ t.throttleCpuUsage(); });   // private; returns early
}
TEST(IndexTaskTest, D0DestructorPath)
{
    auto *ptr = new IndexTask(IndexTask::Type::Create, "/tmp/heap", noopHandler());
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}
TEST(IndexTaskTest, DoTaskWithHandler)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    // doTask() runs the handler in the current thread (no thread check here);
    // it returns HandlerResult{false,...} because noopHandler returns default.
    EXPECT_NO_FATAL_FAILURE({ t.doTask(); });   // private
}
TEST(IndexTaskTest, DoTaskWithNullHandler)
{
    TaskHandler nullHandler;
    IndexTask t(IndexTask::Type::Create, "/tmp", nullHandler);
    EXPECT_NO_FATAL_FAILURE({ t.doTask(); });   // m_handler is null -> result stays default
}
TEST(IndexTaskTest, SetSilentThenThrottleCpuUsage)
{
    IndexTask t(IndexTask::Type::Create, "/tmp", noopHandler());
    t.setSilent(true);
    EXPECT_TRUE(t.silent());
    EXPECT_NO_FATAL_FAILURE({ t.throttleCpuUsage(); });   // tries systemd (fails in sandbox)
}
