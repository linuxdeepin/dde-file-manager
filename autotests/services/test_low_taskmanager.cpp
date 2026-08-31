// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_taskmanager.cpp
 * @brief Unit tests for TaskManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/taskmanager.h"

#include <QTest>

using namespace src;

class TaskManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskManagerTest, TaskManager)
{
    // Test constructor: TaskManager((const IndexContext *context, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TaskManagerTest, cleanupTask)
{
    // Test method: void cleanupTask(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanupTask());
}

TEST_F(TaskManagerTest, currentTaskPath)
{
    // Test getter: std::optional<QString> currentTaskPath()
    auto result = obj->currentTaskPath();
    EXPECT_FALSE(result.has_value());

}

TEST_F(TaskManagerTest, currentTaskType)
{
    // Test getter: std::optional<IndexTask::Type> currentTaskType()
    auto result = obj->currentTaskType();
    EXPECT_FALSE(result.has_value());

}

TEST_F(TaskManagerTest, enqueueCompensationTask)
{
    // Test method: bool enqueueCompensationTask((const QStringList &paths, bool silent))
    QStringList _arg0{};
    auto result = obj->enqueueCompensationTask(_arg0, false);
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, handleRootPathFailure)
{
    // Test method: void handleRootPathFailure((bool success, bool interrupted, const QString &taskPath))
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRootPathFailure(false, false, _arg2));
}

TEST_F(TaskManagerTest, hasQueuedTasks)
{
    // Test bool getter: hasQueuedTasks()
    bool result = obj->hasQueuedTasks();
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, hasRunningTask)
{
    // Test bool getter: hasRunningTask()
    bool result = obj->hasRunningTask();
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, isFullScanTask)
{
    // Test method: bool isFullScanTask((IndexTask::Type type))
    auto result = obj->isFullScanTask(IndexTask::Type());
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, isRecoveryPending)
{
    // Test bool getter: isRecoveryPending()
    bool result = obj->isRecoveryPending();
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, onTaskFinished)
{
    // Test method: void onTaskFinished((IndexTask::Type type, HandlerResult result))
    EXPECT_NO_FATAL_FAILURE(obj->onTaskFinished(IndexTask::Type(), HandlerResult()));
}

TEST_F(TaskManagerTest, onTaskProgress)
{
    // Test method: void onTaskProgress((IndexTask::Type type, qint64 count, qint64 total))
    EXPECT_NO_FATAL_FAILURE(obj->onTaskProgress(IndexTask::Type(), 0, 0));
}

TEST_F(TaskManagerTest, setRecoveryPending)
{
    // Test setter: void setRecoveryPending((bool pending))
    EXPECT_NO_FATAL_FAILURE(obj->setRecoveryPending(false));
}

TEST_F(TaskManagerTest, startNextTask)
{
    // Test bool getter: startNextTask()
    bool result = obj->startNextTask();
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, stopCurrentTask)
{
    // Test method: void stopCurrentTask(())
    EXPECT_NO_FATAL_FAILURE(obj->stopCurrentTask());
}

TEST_F(TaskManagerTest, TaskManager_Destructor)
{
    // Test method:  ~TaskManager(())
    EXPECT_NO_FATAL_FAILURE({ TaskManager *tmp = new TaskManager(); delete tmp; });
}
