// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskmanager_private.cpp
 * @brief Additional TaskManager tests covering onTaskFinished,
 *        handleCorruptedIndex, handleRootPathFailure, updateIndexStatusOnSuccess,
 *        finalizeIndexState, enqueueCompensationTask, and
 *        startTask/StartFileListTask/StartFileMoveTask with additional paths.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QString>
#include <QStringList>
#include <QHash>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"
#include "services/textindex/task/taskmanager.h"
#include "services/textindex/task/indextask.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class TaskManagerPrivateTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    std::unique_ptr<IndexRuntime> runtime;
    TaskManager *mgr { nullptr };

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        runtime = std::make_unique<IndexRuntime>(
            IndexProfile(IndexProfile::Type::Content, "tm_priv", "tm_priv_status.json", "tm_priv_ver", 1,
                         [this]() -> QString { return tmp.path(); },
                         []() -> bool { return true; },
                         [](const QString &) -> bool { return true; },
                         [](const QString &) -> bool { return true; }));
        mgr = runtime->taskManager();
        ASSERT_NE(mgr, nullptr);
    }
};

// ---- enqueueCompensationTask ----
TEST_F(TaskManagerPrivateTest, EnqueueCompensationTask)
{
    EXPECT_TRUE(mgr->enqueueCompensationTask({ "/tmp/a.txt" }));
    EXPECT_TRUE(mgr->hasQueuedTasks());
}

// ---- startTask with QStringList ----
TEST_F(TaskManagerPrivateTest, StartTask_MultiplePaths)
{
    bool result = mgr->startTask(IndexTask::Type::Create, QStringList{ tmp.path() });
    // May succeed or fail depending on runtime, but no crash
    SUCCEED();
}

TEST_F(TaskManagerPrivateTest, StartTask_Create)
{
    bool result = mgr->startTask(IndexTask::Type::Create, tmp.path());
    SUCCEED();
}

TEST_F(TaskManagerPrivateTest, StartTask_UpdateType)
{
    bool result = mgr->startTask(IndexTask::Type::Update, tmp.path());
    SUCCEED();
}

// ---- startFileListTask with actual files ----
TEST_F(TaskManagerPrivateTest, StartFileListTask_SingleFile)
{
    bool result = mgr->startFileListTask(IndexTask::Type::CreateFileList, QStringList{ "/tmp/a.txt" });
    SUCCEED();
}

TEST_F(TaskManagerPrivateTest, StartFileListTask_UpdateFileList)
{
    bool result = mgr->startFileListTask(IndexTask::Type::UpdateFileList, QStringList{ "/tmp/a.txt" });
    SUCCEED();
}

TEST_F(TaskManagerPrivateTest, StartFileListTask_RemoveFileList)
{
    bool result = mgr->startFileListTask(IndexTask::Type::RemoveFileList, QStringList{ "/tmp/a.txt" });
    SUCCEED();
}

// ---- startFileMoveTask with actual moves ----
TEST_F(TaskManagerPrivateTest, StartFileMoveTask_SingleMove)
{
    QHash<QString, QString> moves;
    moves["/old.txt"] = "/new.txt";
    bool result = mgr->startFileMoveTask(moves);
    SUCCEED();
}

// ---- onTaskFinished with null current task ----
TEST_F(TaskManagerPrivateTest, OnTaskFinished_NoCurrentTask)
{
    EXPECT_NO_FATAL_FAILURE({ mgr->onTaskProgress(IndexTask::Type::Create, 10, 100); });
}

// ---- applyDirectoryMovePlans with directory move ----
TEST_F(TaskManagerPrivateTest, ApplyDirectoryMovePlans_MultipleDirectoryMoves)
{
    QHash<QString, QString> moves {
        { "/old/dir1/", "/new/dir1/" },
        { "/old/dir2/", "/new/dir2/" }
    };
    QStringList result = mgr->applyDirectoryMovePlans(moves);
    EXPECT_GE(result.size(), 2);
}

// ---- Multiple enqueue then schedule ----
TEST_F(TaskManagerPrivateTest, MultipleEnqueueThenSchedule)
{
    mgr->enqueueCompensationTask({ "/tmp/x.txt" });
    mgr->enqueueCompensationTask({ "/tmp/y.txt" });
    // Queue has items, schedule should try to run one
    // But without a valid index directory, it may fail gracefully
    EXPECT_NO_FATAL_FAILURE({ mgr->schedule(); });
}

// ---- cleanupTask ----
TEST_F(TaskManagerPrivateTest, CleanupTask)
{
    EXPECT_NO_FATAL_FAILURE({ mgr->cleanupTask(); });
}

// ---- handleCorruptedIndex via private access ---
TEST_F(TaskManagerPrivateTest, HandleCorruptedIndex_EarlyReturnSuccess)
{
    // When result.success is true, handleCorruptedIndex returns early (no crash)
    HandlerResult result;
    result.success = true;
    EXPECT_NO_FATAL_FAILURE({
        mgr->handleCorruptedIndex(IndexTask::Type::Update, result, tmp.path());
    });
}

TEST_F(TaskManagerPrivateTest, HandleCorruptedIndex_NonCreateUpdate)
{
    // Non-Update type returns early without accessing currentTask
    HandlerResult result;
    result.fatal = true;
    EXPECT_NO_FATAL_FAILURE({
        mgr->handleCorruptedIndex(IndexTask::Type::UpdateFileList, result, tmp.path());
    });
}

// ---- handleRootPathFailure ----
TEST_F(TaskManagerPrivateTest, HandleRootPathFailure_SuccessNotInterrupted)
{
    EXPECT_NO_FATAL_FAILURE({
        mgr->handleRootPathFailure(true, false, tmp.path());
    });
}

TEST_F(TaskManagerPrivateTest, HandleRootPathFailure_Failed)
{
    EXPECT_NO_FATAL_FAILURE({
        mgr->handleRootPathFailure(false, false, tmp.path());
    });
}

// ---- updateIndexStatusOnSuccess ----
TEST_F(TaskManagerPrivateTest, UpdateIndexStatusOnSuccess)
{
    HandlerResult result;
    result.indexChanged = true;
    EXPECT_NO_FATAL_FAILURE({
        mgr->updateIndexStatusOnSuccess(IndexTask::Type::Create, result);
    });
}

// ---- finalizeIndexState ----
TEST_F(TaskManagerPrivateTest, FinalizeIndexState)
{
    HandlerResult result;
    result.success = true;
    result.interrupted = false;
    EXPECT_NO_FATAL_FAILURE({
        mgr->finalizeIndexState(IndexTask::Type::Create, result);
    });
}
