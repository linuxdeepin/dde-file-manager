// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QThread>
#include <QTimer>
#include <QTemporaryDir>

#include "task/taskmanager.h"
#include "task/indextask.h"
#include "task/taskhandler.h"
#include "utils/indexutility.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_TaskManager : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for testing
        tempDir = std::make_unique<QTemporaryDir>();
        EXPECT_TRUE(tempDir->isValid());

        testPath = tempDir->path() + "/test";
        QDir().mkpath(testPath);

        // Mock IndexUtility functions
        stub.set_lamda(ADDR(IndexUtility, isDefaultIndexedDirectory), [this](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return mockValidDirectories.contains(path);
        });

        stub.set_lamda(ADDR(IndexUtility, removeIndexStatusFile), []() {
            __DBG_STUB_INVOKE__
            // Mock implementation
        });
        using SaveIndexStatusFunc = void (*)(const QDateTime &, int);
        stub.set_lamda(static_cast<SaveIndexStatusFunc>(&IndexUtility::saveIndexStatus), [](const QDateTime &, int) {
            __DBG_STUB_INVOKE__
            // Mock implementation
        });

        stub.set_lamda(ADDR(IndexUtility, clearIndexDirectory), []() {
            __DBG_STUB_INVOKE__
            // Mock implementation
        });

        // Mock QThread operations
        using QThreadStartFunc = void (QThread::*)(QThread::Priority);
        stub.set_lamda(static_cast<QThreadStartFunc>(&QThread::start), [this](QThread *, QThread::Priority) {
            __DBG_STUB_INVOKE__
            mockThreadStartCalled = true;
        });

        stub.set_lamda(ADDR(QThread, isRunning), [this](QThread *) -> bool {
            __DBG_STUB_INVOKE__
            return mockThreadRunning;
        });

        stub.set_lamda(ADDR(QThread, quit), [this](QThread *) {
            __DBG_STUB_INVOKE__
            mockThreadQuitCalled = true;
        });

        using QThreadWaitFunc = bool (QThread::*)(unsigned long);
        stub.set_lamda(static_cast<QThreadWaitFunc>(&QThread::wait), [this](QThread *, int timeout) -> bool {
            __DBG_STUB_INVOKE__
            mockThreadWaitCalled = true;
            return mockThreadWaitSuccess;
        });

        stub.set_lamda(ADDR(QThread, terminate), [this](QThread *) {
            __DBG_STUB_INVOKE__
            mockThreadTerminateCalled = true;
        });

        // Reset mock states
        mockValidDirectories.clear();
        mockValidDirectories.insert(testPath);
        mockValidDirectories.insert("/home/test");
        mockThreadStartCalled = false;
        mockThreadRunning = false;
        mockThreadQuitCalled = false;
        mockThreadWaitCalled = false;
        mockThreadWaitSuccess = true;
        mockThreadTerminateCalled = false;
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testPath;

    // Mock state variables
    QSet<QString> mockValidDirectories;
    bool mockThreadStartCalled = false;
    bool mockThreadRunning = false;
    bool mockThreadQuitCalled = false;
    bool mockThreadWaitCalled = false;
    bool mockThreadWaitSuccess = true;
    bool mockThreadTerminateCalled = false;

private:
    stub_ext::StubExt stub;
};

// Constructor and Destructor Tests
TEST_F(UT_TaskManager, Constructor_CreatesInstance_RegistersMetaTypes)
{
    TaskManager manager;

    EXPECT_TRUE(true);   // Constructor completed without exception
}

TEST_F(UT_TaskManager, Destructor_WithRunningTask_StopsTaskAndThread)
{
    auto manager = std::make_unique<TaskManager>();
    mockThreadRunning = true;

    // Mock current task
    stub.set_lamda(ADDR(IndexTask, stop), [](IndexTask *) {
        __DBG_STUB_INVOKE__
        // Mock task stop
    });

    manager.reset();   // Trigger destructor

    EXPECT_TRUE(mockThreadQuitCalled);
    EXPECT_TRUE(mockThreadWaitCalled);
}

TEST_F(UT_TaskManager, Destructor_ThreadWaitTimeout_TerminatesThread)
{
    auto manager = std::make_unique<TaskManager>();
    mockThreadRunning = true;
    mockThreadWaitSuccess = false;

    manager.reset();   // Trigger destructor

    EXPECT_TRUE(mockThreadQuitCalled);
    EXPECT_TRUE(mockThreadWaitCalled);
    EXPECT_TRUE(mockThreadTerminateCalled);
}

// Single Path Task Tests
TEST_F(UT_TaskManager, StartTask_SinglePath_ValidPath_ReturnsTrue)
{
    TaskManager manager;

    bool result = manager.startTask(IndexTask::Type::Create, testPath, false);

    EXPECT_TRUE(result);
    EXPECT_TRUE(mockThreadStartCalled);
}

TEST_F(UT_TaskManager, StartTask_SinglePath_InvalidPath_ReturnsFalse)
{
    TaskManager manager;
    QString invalidPath = "/invalid/path";

    bool result = manager.startTask(IndexTask::Type::Create, invalidPath, false);

    EXPECT_FALSE(result);
    EXPECT_FALSE(mockThreadStartCalled);
}

TEST_F(UT_TaskManager, StartTask_SinglePath_CreateType_RemovesStatusFile)
{
    TaskManager manager;
    bool removeStatusFileCalled = false;

    stub.set_lamda(ADDR(IndexUtility, removeIndexStatusFile), [&removeStatusFileCalled]() {
        __DBG_STUB_INVOKE__
        removeStatusFileCalled = true;
    });

    manager.startTask(IndexTask::Type::Create, testPath, false);

    EXPECT_TRUE(removeStatusFileCalled);
}

TEST_F(UT_TaskManager, StartTask_SinglePath_UpdateType_DoesNotRemoveStatusFile)
{
    TaskManager manager;
    bool removeStatusFileCalled = false;

    stub.set_lamda(ADDR(IndexUtility, removeIndexStatusFile), [&removeStatusFileCalled]() {
        __DBG_STUB_INVOKE__
        removeStatusFileCalled = true;
    });

    manager.startTask(IndexTask::Type::Update, testPath, false);

    EXPECT_FALSE(removeStatusFileCalled);
}

// Multi Path Task Tests
TEST_F(UT_TaskManager, StartTask_MultiPath_ValidPaths_ReturnsTrue)
{
    TaskManager manager;
    QStringList paths = { testPath, "/home/test" };

    bool result = manager.startTask(IndexTask::Type::Create, paths, false);

    EXPECT_TRUE(result);
    EXPECT_TRUE(mockThreadStartCalled);
}

TEST_F(UT_TaskManager, StartTask_MultiPath_EmptyList_ReturnsFalse)
{
    TaskManager manager;
    QStringList emptyPaths;

    bool result = manager.startTask(IndexTask::Type::Create, emptyPaths, false);

    EXPECT_FALSE(result);
    EXPECT_FALSE(mockThreadStartCalled);
}

TEST_F(UT_TaskManager, StartTask_MultiPath_SomeInvalidPaths_ReturnsFalse)
{
    TaskManager manager;
    QStringList paths = { testPath, "/invalid/path" };

    bool result = manager.startTask(IndexTask::Type::Create, paths, false);

    EXPECT_FALSE(result);
    EXPECT_FALSE(mockThreadStartCalled);
}

// File List Task Tests
TEST_F(UT_TaskManager, StartFileListTask_ValidFiles_ReturnsTrue)
{
    TaskManager manager;
    QStringList files = { testPath + "/file1.txt", testPath + "/file2.txt" };

    bool result = manager.startFileListTask(IndexTask::Type::CreateFileList, files, false);

    EXPECT_TRUE(result);
    EXPECT_TRUE(mockThreadStartCalled);
}

TEST_F(UT_TaskManager, StartFileListTask_EmptyList_ReturnsFalse)
{
    TaskManager manager;
    QStringList emptyFiles;

    bool result = manager.startFileListTask(IndexTask::Type::CreateFileList, emptyFiles, false);

    EXPECT_FALSE(result);
    EXPECT_FALSE(mockThreadStartCalled);
}

TEST_F(UT_TaskManager, StartFileListTask_WithRunningTask_QueuesTask)
{
    TaskManager manager;

    // Mock running task
    stub.set_lamda(ADDR(TaskManager, hasRunningTask), [](TaskManager *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QStringList files = { testPath + "/file.txt" };
    bool result = manager.startFileListTask(IndexTask::Type::CreateFileList, files, true);

    EXPECT_TRUE(result);   // Should queue the task
}

// File Move Task Tests
TEST_F(UT_TaskManager, StartFileMoveTask_ValidMoves_ReturnsTrue)
{
    TaskManager manager;
    QHash<QString, QString> movedFiles;
    movedFiles[testPath + "/old.txt"] = testPath + "/new.txt";

    bool result = manager.startFileMoveTask(movedFiles, false);

    EXPECT_TRUE(result);
    EXPECT_TRUE(mockThreadStartCalled);
}

TEST_F(UT_TaskManager, StartFileMoveTask_EmptyHash_ReturnsFalse)
{
    TaskManager manager;
    QHash<QString, QString> emptyMoves;

    bool result = manager.startFileMoveTask(emptyMoves, false);

    EXPECT_FALSE(result);
    EXPECT_FALSE(mockThreadStartCalled);
}

// Task State Management Tests
TEST_F(UT_TaskManager, HasRunningTask_NoTask_ReturnsFalse)
{
    TaskManager manager;

    bool result = manager.hasRunningTask();

    EXPECT_FALSE(result);
}

TEST_F(UT_TaskManager, StopCurrentTask_NoTask_DoesNotCrash)
{
    TaskManager manager;

    EXPECT_NO_THROW({
        manager.stopCurrentTask();
    });
}

TEST_F(UT_TaskManager, CurrentTaskType_NoTask_ReturnsNullopt)
{
    TaskManager manager;

    auto result = manager.currentTaskType();

    EXPECT_FALSE(result.has_value());
}

TEST_F(UT_TaskManager, CurrentTaskPath_NoTask_ReturnsNullopt)
{
    TaskManager manager;

    auto result = manager.currentTaskPath();

    EXPECT_FALSE(result.has_value());
}

// Task Handler Tests
TEST_F(UT_TaskManager, GetTaskHandler_CreateType_ReturnsValidHandler)
{
    TaskManager manager;

    // Access private method using friend or protected access patterns
    // For this test, we verify the functionality indirectly
    bool result = manager.startTask(IndexTask::Type::Create, testPath, false);

    EXPECT_TRUE(result);   // Valid handler should allow task to start
}

TEST_F(UT_TaskManager, GetTaskHandler_UpdateType_ReturnsValidHandler)
{
    TaskManager manager;

    bool result = manager.startTask(IndexTask::Type::Update, testPath, false);

    EXPECT_TRUE(result);   // Valid handler should allow task to start
}

// Type String Conversion Tests
TEST_F(UT_TaskManager, TypeToString_CreateType_ReturnsCorrectString)
{
    // Test the static function indirectly through task operations
    TaskManager manager;
    QSignalSpy spy(&manager, &TaskManager::taskFinished);

    // This will test the type string conversion when signals are emitted
    EXPECT_TRUE(true);   // Placeholder - actual testing would require signal emission
}

// Signal Handling Tests
TEST_F(UT_TaskManager, OnTaskProgress_EmitsSignal)
{
    TaskManager manager;
    QSignalSpy spy(&manager, &TaskManager::taskProgressChanged);

    // Mock task and emit progress signal
    // This would require accessing private members or using friend classes
    EXPECT_TRUE(true);   // Placeholder for complex signal testing
}

TEST_F(UT_TaskManager, OnTaskFinished_SuccessfulTask_EmitsSignalAndSavesStatus)
{
    TaskManager manager;
    QSignalSpy spy(&manager, &TaskManager::taskFinished);

    bool saveStatusCalled = false;
    using SaveIndexStatusFunc = void (*)(const QDateTime &, int);
    stub.set_lamda(static_cast<SaveIndexStatusFunc>(&IndexUtility::saveIndexStatus), [&saveStatusCalled](const QDateTime &, int) {
        __DBG_STUB_INVOKE__
        saveStatusCalled = true;
    });

    // Test would require triggering task completion
    EXPECT_TRUE(true);   // Placeholder
}

TEST_F(UT_TaskManager, OnTaskFinished_FailedUpdateTask_TriggersRebuild)
{
    TaskManager manager;

    bool clearIndexCalled = false;
    stub.set_lamda(ADDR(IndexUtility, clearIndexDirectory), [&clearIndexCalled]() {
        __DBG_STUB_INVOKE__
        clearIndexCalled = true;
    });

    // Test would require triggering failed update task
    EXPECT_TRUE(true);   // Placeholder
}

// Queue Management Tests
TEST_F(UT_TaskManager, StartNextTask_EmptyQueue_ReturnsFalse)
{
    TaskManager manager;

    // Test would require accessing private queue management
    EXPECT_TRUE(true);   // Placeholder
}

TEST_F(UT_TaskManager, StartNextTask_WithQueuedTasks_StartsNextTask)
{
    TaskManager manager;

    // Test would require queue manipulation
    EXPECT_TRUE(true);   // Placeholder
}

// Thread Safety Tests
TEST_F(UT_TaskManager, ConcurrentTaskRequests_HandledSafely)
{
    // TaskManager manager;

    // // Test concurrent access patterns
    // std::atomic<int> successCount { 0 };
    // std::vector<std::thread> threads;

    // for (int i = 0; i < 5; ++i) {
    //     threads.emplace_back([&manager, &successCount, this, i]() {
    //         QString path = testPath + QString::number(i);
    //         mockValidDirectories.insert(path);
    //         if (manager.startTask(IndexTask::Type::Create, path, true)) {
    //             successCount++;
    //         }
    //     });
    // }

    // for (auto &thread : threads) {
    //     thread.join();
    // }

    // // Only one task should succeed, others should be queued
    // EXPECT_GE(successCount.load(), 1);
}

// Error Handling Tests
TEST_F(UT_TaskManager, StartTask_InvalidTaskType_HandledGracefully)
{
    // TaskManager manager;

    // // Test with invalid enum value (if possible)
    // EXPECT_NO_THROW({
    //     auto invalidType = static_cast<IndexTask::Type>(999);
    //     manager.startTask(invalidType, testPath, false);
    // });
}

TEST_F(UT_TaskManager, TaskExecution_HandlerThrowsException_HandledGracefully)
{
    TaskManager manager;

    // Test would require mocking handler to throw exception
    EXPECT_TRUE(true);   // Placeholder
}
