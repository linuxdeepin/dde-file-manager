// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskmanager_safe.cpp
 * @brief Unit tests for TaskManager (task/taskmanager.cpp) — the safe,
 *        dependency-light subset: query accessors, recovery flag, queue
 *        inspection, task lifecycle no-ops and the handler/toString helpers.
 *        Heavy start-task / DBus paths are intentionally avoided.
 *
 *        Private members are accessed directly (the test build uses
 *        -fno-access-control).
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QString>
#include <QStringList>
#include <optional>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"
#include "services/textindex/task/taskmanager.h"
#include "services/textindex/task/indextask.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class TaskManagerSafeTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    IndexProfile makeProfile()
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "tm_test",
                            "tm_status.json",
                            "tm_version",
                            1,
                            [this]() -> QString { return tmp.path(); },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &) -> bool { return true; });
    }

    std::unique_ptr<IndexRuntime> runtime;
    TaskManager *mgr { nullptr };

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        runtime = std::make_unique<IndexRuntime>(makeProfile());
        mgr = runtime->taskManager();
        ASSERT_NE(mgr, nullptr);
    }
};

TEST_F(TaskManagerSafeTest, NoRunningOrQueuedTaskByDefault)
{
    EXPECT_FALSE(mgr->hasRunningTask());
    EXPECT_FALSE(mgr->hasQueuedTasks());
}

TEST_F(TaskManagerSafeTest, CurrentTaskTypeAndPathAreNullopt)
{
    EXPECT_FALSE(mgr->currentTaskType().has_value());
    EXPECT_FALSE(mgr->currentTaskPath().has_value());
}

TEST_F(TaskManagerSafeTest, StopCurrentTaskAndCleanupAreNoOps)
{
    EXPECT_NO_FATAL_FAILURE({ mgr->stopCurrentTask(); });
    EXPECT_NO_FATAL_FAILURE({ mgr->cleanupTask(); });   // private
}

TEST_F(TaskManagerSafeTest, ScheduleOnEmptyQueueIsNoOp)
{
    EXPECT_NO_FATAL_FAILURE({ mgr->schedule(); });   // private
}

TEST_F(TaskManagerSafeTest, RecoveryFlagRoundtrip)
{
    EXPECT_FALSE(mgr->isRecoveryPending());
    mgr->setRecoveryPending(true);
    EXPECT_TRUE(mgr->isRecoveryPending());
    mgr->setRecoveryPending(false);
    EXPECT_FALSE(mgr->isRecoveryPending());
}

TEST_F(TaskManagerSafeTest, GetTaskHandlerReturnsNonNullForCreateAndUpdate)
{
    EXPECT_NE(mgr->getTaskHandler(IndexTask::Type::Create), nullptr);   // private
    EXPECT_NE(mgr->getTaskHandler(IndexTask::Type::Update), nullptr);   // private
}

TEST_F(TaskManagerSafeTest, GetTaskHandlerReturnsNullForUnknownType)
{
    // A type outside the handled enum range returns nullptr.
    EXPECT_EQ(mgr->getTaskHandler(static_cast<IndexTask::Type>(999)), nullptr);   // private
}

TEST_F(TaskManagerSafeTest, TypeToStringCoversAllKnownTypes)
{
    EXPECT_EQ(mgr->typeToString(IndexTask::Type::Create), QString("create"));
    EXPECT_EQ(mgr->typeToString(IndexTask::Type::Update), QString("update"));
    EXPECT_EQ(mgr->typeToString(IndexTask::Type::CreateFileList), QString("create-file-list"));
    EXPECT_EQ(mgr->typeToString(IndexTask::Type::UpdateFileList), QString("update-file-list"));
    EXPECT_EQ(mgr->typeToString(IndexTask::Type::RemoveFileList), QString("remove-file-list"));
    EXPECT_EQ(mgr->typeToString(IndexTask::Type::MoveFileList), QString("move-file-list"));
    EXPECT_EQ(mgr->typeToString(static_cast<IndexTask::Type>(999)), QString("unknown"));
}

TEST_F(TaskManagerSafeTest, IsFullScanTaskClassification)
{
    EXPECT_TRUE(mgr->isFullScanTask(IndexTask::Type::Create));
    EXPECT_TRUE(mgr->isFullScanTask(IndexTask::Type::Update));
    EXPECT_FALSE(mgr->isFullScanTask(IndexTask::Type::CreateFileList));
    EXPECT_FALSE(mgr->isFullScanTask(IndexTask::Type::UpdateFileList));
    EXPECT_FALSE(mgr->isFullScanTask(IndexTask::Type::RemoveFileList));
    EXPECT_FALSE(mgr->isFullScanTask(IndexTask::Type::MoveFileList));
}
