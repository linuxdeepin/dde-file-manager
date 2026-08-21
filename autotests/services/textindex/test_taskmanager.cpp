// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskmanager.cpp
 * @brief Additional TaskManager tests: enqueueCompensationTask (empty/valid),
 *        applyDirectoryMovePlans (empty/no-directory-move/single-directory),
 *        onTaskProgress with null currentTask, startTask with empty pathList,
 *        startFileListTask with empty fileList, startFileMoveTask with empty
 *        move map — all early-return paths without spawning threads.
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

class TaskManagerTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    std::unique_ptr<IndexRuntime> runtime;
    TaskManager *mgr { nullptr };

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        runtime = std::make_unique<IndexRuntime>(
            IndexProfile(IndexProfile::Type::Content, "tm13", "tm13_status.json", "tm13_ver", 1,
                         [this]() -> QString { return tmp.path(); },
                         []() -> bool { return true; },
                         [](const QString &) -> bool { return true; },
                         [](const QString &) -> bool { return true; }));
        mgr = runtime->taskManager();
        ASSERT_NE(mgr, nullptr);
    }
};

TEST_F(TaskManagerTest, EnqueueCompensationTaskEmptyPathsReturnsFalse)
{
    EXPECT_FALSE(mgr->enqueueCompensationTask({}));
}

TEST_F(TaskManagerTest, EnqueueCompensationTaskWithPathsReturnsTrue)
{
    EXPECT_TRUE(mgr->enqueueCompensationTask({"/tmp/a.txt"}));
    EXPECT_TRUE(mgr->hasQueuedTasks());
}

TEST_F(TaskManagerTest, ApplyDirectoryMovePlansEmptyReturnsEmpty)
{
    QHash<QString, QString> empty;
    QStringList result = mgr->applyDirectoryMovePlans(empty);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TaskManagerTest, ApplyDirectoryMovePlansNonDirectoryReturnsEmpty)
{
    QHash<QString, QString> moves { {"/old/file.txt", "/new/file.txt"} };
    QStringList result = mgr->applyDirectoryMovePlans(moves);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TaskManagerTest, ApplyDirectoryMovePlansDirectoryMove)
{
    QHash<QString, QString> moves { {"/old/dir/", "/new/dir/"} };
    QStringList result = mgr->applyDirectoryMovePlans(moves);
    // compensationPaths has at least /new/dir/
    EXPECT_GE(result.size(), 1);
}

TEST_F(TaskManagerTest, OnTaskProgressWithNullTaskIsNoOp)
{
    EXPECT_NO_FATAL_FAILURE({ mgr->onTaskProgress(IndexTask::Type::Create, 10, 100); });
}

TEST_F(TaskManagerTest, StartTaskEmptyPathListReturnsFalse)
{
    // startTask(QString path) with empty path should fail
    EXPECT_FALSE(mgr->startTask(IndexTask::Type::Create, QString()));
}

TEST_F(TaskManagerTest, StartFileListTaskEmptyListReturnsFalse)
{
    EXPECT_FALSE(mgr->startFileListTask(IndexTask::Type::CreateFileList, QStringList()));
}

TEST_F(TaskManagerTest, StartFileMoveTaskEmptyMovesReturnsFalse)
{
    QHash<QString, QString> empty;
    EXPECT_FALSE(mgr->startFileMoveTask(empty));
}
