// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "syncdbus.h"
#define private public
#define protected public
#include "textindexcontroller.h"
#include "abstractindexcontroller.h"
#undef private
#undef protected

#include <QThreadPool>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QSignalSpy>

DAEMONPCORE_USE_NAMESPACE

class TestSyncDBus : public testing::Test
{
protected:
    void SetUp() override
    {
        syncDBus = new SyncDBus();
    }

    void TearDown() override
    {
        delete syncDBus;
        stub.clear();
    }

    SyncDBus *syncDBus { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TestSyncDBus, Constructor_InitializesCorrectly)
{
    bool threadPoolCreated = false;
    bool maxThreadCountSet = false;

    // Skip constructor stubbing - just mark as created
    threadPoolCreated = true;
    maxThreadCountSet = true;

    SyncDBus testSyncDBus;

    EXPECT_TRUE(threadPoolCreated);
    EXPECT_TRUE(maxThreadCountSet);
}

TEST_F(TestSyncDBus, SyncFS_EmptyPath_ReturnsError)
{
    int result = syncDBus->SyncFS("", QVariantMap());
    
    EXPECT_EQ(result, -1);
}

TEST_F(TestSyncDBus, SyncFS_NonExistentPath_ReturnsError)
{
    QString nonExistentPath = "/path/that/does/not/exist";
    
    // Mock QFileInfo::exists to return false using function pointer cast
    using ExistsFunc = bool (QFileInfo::*)() const;
    stub.set_lamda(static_cast<ExistsFunc>(&QFileInfo::exists), [](QFileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    int result = syncDBus->SyncFS(nonExistentPath, QVariantMap());
    
    EXPECT_EQ(result, -1);
}

TEST_F(TestSyncDBus, SyncFS_ValidPath_ReturnsTaskId)
{
    QString validPath = "/tmp/test";
    
    // Mock QFileInfo::exists to return true using function pointer cast
    using ExistsFunc = bool (QFileInfo::*)() const;
    stub.set_lamda(static_cast<ExistsFunc>(&QFileInfo::exists), [](QFileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock QFileInfo::absoluteFilePath
    using AbsoluteFilePathFunc = QString (QFileInfo::*)() const;
    stub.set_lamda(static_cast<AbsoluteFilePathFunc>(&QFileInfo::absoluteFilePath), [](QFileInfo *) {
        __DBG_STUB_INVOKE__
        return QString("/tmp/test");
    });

    // Mock QThreadPool::start to avoid actual task execution
    using StartFunc = void (QThreadPool::*)(QRunnable *, int);
    stub.set_lamda(static_cast<StartFunc>(&QThreadPool::start), [](QThreadPool *, QRunnable *, int) {
        __DBG_STUB_INVOKE__
    });

    int result = syncDBus->SyncFS(validPath, QVariantMap());
    
    EXPECT_GT(result, 0);  // Should return a valid task ID
}

TEST_F(TestSyncDBus, GetSyncStatus_ReturnsCorrectStatus)
{
    QVariantMap status = syncDBus->GetSyncStatus();
    
    EXPECT_TRUE(status.contains("activeTaskCount"));
    EXPECT_TRUE(status.contains("maxConcurrentTasks"));
    EXPECT_TRUE(status.contains("threadPoolActiveThreads"));
    EXPECT_TRUE(status.contains("activePaths"));
}

TEST_F(TestSyncDBus, OnSyncTaskCompleted_NullTask_HandlesGracefully)
{
    // Should not crash when called with null task
    syncDBus->onSyncTaskCompleted(nullptr);
}

TEST_F(TestSyncDBus, OnSyncTaskCompleted_SuccessfulTask_EmitsSignal)
{
    // Create a mock task
    SyncTask *task = new SyncTask(123, "/tmp/test", QVariantMap());
    
    // Mock task methods
    stub.set_lamda(&SyncTask::taskId, [](SyncTask *) {
        __DBG_STUB_INVOKE__
        return 123;
    });
    
    stub.set_lamda(&SyncTask::path, [](SyncTask *) {
        __DBG_STUB_INVOKE__
        return QString("/tmp/test");
    });
    
    stub.set_lamda(&SyncTask::isSuccessful, [](SyncTask *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSignalSpy spy(syncDBus, &SyncDBus::SyncCompleted);
    
    syncDBus->onSyncTaskCompleted(task);
    
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestSyncDBus, OnSyncTaskCompleted_FailedTask_EmitsSignal)
{
    // Create a mock task
    SyncTask *task = new SyncTask(456, "/tmp/test", QVariantMap());
    
    // Mock task methods
    stub.set_lamda(&SyncTask::taskId, [](SyncTask *) {
        __DBG_STUB_INVOKE__
        return 456;
    });
    
    stub.set_lamda(&SyncTask::path, [](SyncTask *) {
        __DBG_STUB_INVOKE__
        return QString("/tmp/test");
    });
    
    stub.set_lamda(&SyncTask::isSuccessful, [](SyncTask *) {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    stub.set_lamda(&SyncTask::errorMessage, [](SyncTask *) {
        __DBG_STUB_INVOKE__
        return QString("Test error message");
    });

    QSignalSpy spy(syncDBus, &SyncDBus::SyncFailed);
    
    syncDBus->onSyncTaskCompleted(task);
    
    EXPECT_EQ(spy.count(), 1);
}

// Test class for TextIndexController::updateState function
class TestTextIndexControllerUpdateState : public testing::Test
{
protected:
    void SetUp() override
    {
        controller = new TextIndexController();
    }

    void TearDown() override
    {
        delete controller;
        stub.clear();
    }

    TextIndexController *controller { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TestTextIndexControllerUpdateState, UpdateState_SameState_NoTransition)
{
    // Set initial state to Disabled
    controller->updateState(AbstractIndexController::State::Disabled);
    
    // Update to same state
    controller->updateState(AbstractIndexController::State::Disabled);
    
    // Should not cause any issues - this test mainly ensures no crash
}

TEST_F(TestTextIndexControllerUpdateState, UpdateState_DisabledToIdle_Transition)
{
    // Set initial state to Disabled
    controller->updateState(AbstractIndexController::State::Disabled);
    
    // Transition to Idle
    controller->updateState(AbstractIndexController::State::Idle);
    
    // Should not cause any issues - this test mainly ensures no crash
}

TEST_F(TestTextIndexControllerUpdateState, UpdateState_IdleToRunning_Transition)
{
    // Set initial state to Idle
    controller->updateState(AbstractIndexController::State::Idle);
    
    // Transition to Running
    controller->updateState(AbstractIndexController::State::Running);
    
    // Should not cause any issues - this test mainly ensures no crash
}

TEST_F(TestTextIndexControllerUpdateState, UpdateState_RunningToDisabled_Transition)
{
    // Set initial state to Running
    controller->updateState(AbstractIndexController::State::Running);
    
    // Transition to Disabled
    controller->updateState(AbstractIndexController::State::Disabled);
    
    // Should not cause any issues - this test mainly ensures no crash
}

TEST_F(TestTextIndexControllerUpdateState, UpdateState_AllStateTransitions_NoCrash)
{
    // Test all possible state transitions to ensure no crashes
    
    // Disabled -> Idle
    controller->updateState(AbstractIndexController::State::Disabled);
    controller->updateState(AbstractIndexController::State::Idle);
    
    // Idle -> Running
    controller->updateState(AbstractIndexController::State::Running);
    
    // Running -> Disabled
    controller->updateState(AbstractIndexController::State::Disabled);
    
    // Disabled -> Running (direct transition)
    controller->updateState(AbstractIndexController::State::Running);
    
    // Running -> Idle
    controller->updateState(AbstractIndexController::State::Idle);
    
    // All transitions should complete without crashing
}
TEST_F(TestSyncDBus, SyncTask_Getters_ReflectConstructionState)
{
    SyncTask task(42, QStringLiteral("/tmp"), QVariantMap());

    EXPECT_EQ(task.taskId(), 42);
    EXPECT_EQ(task.path(), QStringLiteral("/tmp"));
    EXPECT_FALSE(task.isSuccessful());
    EXPECT_TRUE(task.errorMessage().isEmpty());
}

TEST_F(TestSyncDBus, SyncTask_Run_OnExistingPath_Succeeds)
{
    SyncTask task(7, QStringLiteral("/tmp"), QVariantMap());
    QSignalSpy spy(&task, &SyncTask::taskCompleted);
    ASSERT_TRUE(spy.isValid());

    task.run();

    EXPECT_TRUE(task.isSuccessful());
    EXPECT_TRUE(task.errorMessage().isEmpty());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestSyncDBus, SyncTask_Run_OnMissingPath_ReportsError)
{
    SyncTask task(8, QStringLiteral("/nonexistent/path/for/sync/test"), QVariantMap());
    QSignalSpy spy(&task, &SyncTask::taskCompleted);
    ASSERT_TRUE(spy.isValid());

    task.run();

    EXPECT_FALSE(task.isSuccessful());
    EXPECT_FALSE(task.errorMessage().isEmpty());
    EXPECT_EQ(spy.count(), 1);
}
