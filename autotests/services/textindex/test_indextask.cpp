// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QThread>
#include <QCoreApplication>
#include <LuceneException.h>

#include "task/indextask.h"
#include "task/progressnotifier.h"
#include "utils/systemdcpuutils.h"
#include "utils/textindexconfig.h"

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;

class UT_IndexTask : public testing::Test
{
protected:
    void SetUp() override
    {
        // Mock ProgressNotifier
        stub.set_lamda(ADDR(ProgressNotifier, instance), []() -> ProgressNotifier * {
            __DBG_STUB_INVOKE__
            static ProgressNotifier mockInstance;
            return &mockInstance;
        });

        // Mock SystemdCpuUtils
        stub.set_lamda(ADDR(SystemdCpuUtils, setCpuQuota), [this](const QString &serviceName, int percentage, QString *errorMsg) -> bool {
            __DBG_STUB_INVOKE__
            lastCpuQuotaService = serviceName;
            lastCpuQuotaPercentage = percentage;
            return mockCpuQuotaSuccess;
        });

        // Mock TextIndexConfig
        stub.set_lamda(ADDR(TextIndexConfig, instance), []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(ADDR(TextIndexConfig, cpuUsageLimitPercent), [this](TextIndexConfig *) -> int {
            __DBG_STUB_INVOKE__
            return mockCpuLimitPercent;
        });

        // Mock QThread and QCoreApplication
        stub.set_lamda(ADDR(QThread, currentThread), []() -> QThread * {
            __DBG_STUB_INVOKE__
            static QThread mockWorkerThread;
            return &mockWorkerThread;
        });

        stub.set_lamda(ADDR(QCoreApplication, instance), []() -> QCoreApplication * {
            __DBG_STUB_INVOKE__
            static QCoreApplication mockApp(argc, argv);
            return &mockApp;
        });

        stub.set_lamda(ADDR(QObject, thread), [](QObject *) -> QThread * {
            __DBG_STUB_INVOKE__
            static QThread mockMainThread;
            return &mockMainThread;
        });

        // Reset mock states
        mockCpuQuotaSuccess = true;
        mockCpuLimitPercent = 50;
        lastCpuQuotaService.clear();
        lastCpuQuotaPercentage = 0;
        handlerCallCount = 0;
        handlerResult = HandlerResult { true, false, false, false };
    }

    void TearDown() override
    {
        stub.clear();
    }

    // Mock state variables
    bool mockCpuQuotaSuccess = true;
    int mockCpuLimitPercent = 50;
    QString lastCpuQuotaService;
    int lastCpuQuotaPercentage = 0;
    int handlerCallCount = 0;
    HandlerResult handlerResult;

    // Mock argc/argv for QCoreApplication
    static int argc;
    static char *argv[];

private:
    stub_ext::StubExt stub;
};

// Static members definition
int UT_IndexTask::argc = 1;
char *UT_IndexTask::argv[] = { "test", nullptr };

// Constructor Tests
TEST_F(UT_IndexTask, Constructor_ValidParameters_InitializesCorrectly)
{
    auto handler = [this](const QString &path, TaskState &state) -> HandlerResult {
        handlerCallCount++;
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);

    EXPECT_EQ(task.taskType(), IndexTask::Type::Create);
    EXPECT_EQ(task.taskPath(), "/test/path");
    EXPECT_EQ(task.status(), IndexTask::Status::NotStarted);
    EXPECT_FALSE(task.isRunning());
    EXPECT_FALSE(task.silent());
}

TEST_F(UT_IndexTask, Constructor_WithParent_SetsParentCorrectly)
{
    QObject parent;
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Update, "/test/path", handler, &parent);

    EXPECT_EQ(task.parent(), &parent);
}

// Destructor Tests
TEST_F(UT_IndexTask, Destructor_DisconnectsProgressNotifier)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    auto task = std::make_unique<IndexTask>(IndexTask::Type::Create, "/test/path", handler);

    // Should not crash when destroyed
    EXPECT_NO_THROW({
        task.reset();
    });
}

// Silent Mode Tests
TEST_F(UT_IndexTask, SetSilent_True_EnablesSilentMode)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.setSilent(true);

    EXPECT_TRUE(task.silent());
}

TEST_F(UT_IndexTask, SetSilent_False_DisablesSilentMode)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.setSilent(true);
    task.setSilent(false);

    EXPECT_FALSE(task.silent());
}

// CPU Throttling Tests
TEST_F(UT_IndexTask, ThrottleCpuUsage_SilentMode_SetsCpuQuota)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.setSilent(true);

    // Call throttleCpuUsage indirectly through task execution
    task.start();

    EXPECT_EQ(lastCpuQuotaService, Defines::kTextIndexServiceName);
    EXPECT_EQ(lastCpuQuotaPercentage, mockCpuLimitPercent);
}

TEST_F(UT_IndexTask, ThrottleCpuUsage_NonSilentMode_SkipsCpuThrottling)
{
    auto handler = [this](const QString &, TaskState &) -> HandlerResult {
        handlerCallCount++;
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.setSilent(false);

    task.start();

    // CPU quota should not be set in non-silent mode
    EXPECT_TRUE(lastCpuQuotaService.isEmpty());
    EXPECT_EQ(lastCpuQuotaPercentage, 0);
}

TEST_F(UT_IndexTask, ThrottleCpuUsage_SetCpuQuotaFails_ContinuesExecution)
{
    mockCpuQuotaSuccess = false;
    auto handler = [this](const QString &, TaskState &) -> HandlerResult {
        handlerCallCount++;
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.setSilent(true);

    // Should not throw even if CPU quota setting fails
    EXPECT_NO_THROW({
        task.start();
    });

    EXPECT_EQ(handlerCallCount, 1);   // Handler should still be called
}

// Task Execution Tests
TEST_F(UT_IndexTask, Start_AlreadyRunning_IgnoresRequest)
{
    auto handler = [this](const QString &, TaskState &) -> HandlerResult {
        handlerCallCount++;
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);

    // Mock task as running after first start
    static bool firstCall = true;
    stub.set_lamda(ADDR(TaskState, isRunning), [](TaskState *) -> bool {
        __DBG_STUB_INVOKE__
        static bool hasStarted = false;
        if (!hasStarted) {
            hasStarted = true;
            return false;  // First call: not running, allow start
        }
        return true;  // Subsequent calls: running, ignore
    });

    task.start();
    task.start();   // Second call should be ignored

    // Handler should only be called once
    EXPECT_EQ(handlerCallCount, 1);
}

TEST_F(UT_IndexTask, Start_ValidTask_ExecutesHandler)
{
    auto handler = [this](const QString &path, TaskState &state) -> HandlerResult {
        handlerCallCount++;
        EXPECT_EQ(path, "/test/path");
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.start();

    EXPECT_EQ(handlerCallCount, 1);
    EXPECT_EQ(task.status(), IndexTask::Status::Finished);
}

TEST_F(UT_IndexTask, Start_HandlerReturnsFailure_SetsFailedStatus)
{
    handlerResult.success = false;
    auto handler = [this](const QString &, TaskState &) -> HandlerResult {
        handlerCallCount++;
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.start();

    EXPECT_EQ(task.status(), IndexTask::Status::Failed);
}

TEST_F(UT_IndexTask, Start_HandlerReturnsSuccess_SetsFinishedStatus)
{
    handlerResult.success = true;
    auto handler = [this](const QString &, TaskState &) -> HandlerResult {
        handlerCallCount++;
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.start();

    EXPECT_EQ(task.status(), IndexTask::Status::Finished);
}

// Stop Tests
TEST_F(UT_IndexTask, Stop_RunningTask_StopsExecution)
{
    bool taskStateStopped = false;
    stub.set_lamda(ADDR(TaskState, stop), [&taskStateStopped](TaskState *) {
        __DBG_STUB_INVOKE__
        taskStateStopped = true;
    });

    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.stop();

    EXPECT_TRUE(taskStateStopped);
}

// Progress Handling Tests
TEST_F(UT_IndexTask, OnProgressChanged_RunningTask_EmitsSignal)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    QSignalSpy spy(&task, &IndexTask::progressChanged);

    // Mock task as running
    stub.set_lamda(ADDR(TaskState, isRunning), [](TaskState *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Simulate progress update
    task.onProgressChanged(50, 100);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toInt(), static_cast<int>(IndexTask::Type::Create));
    EXPECT_EQ(arguments.at(1).toLongLong(), 50);
    EXPECT_EQ(arguments.at(2).toLongLong(), 100);
}

TEST_F(UT_IndexTask, OnProgressChanged_NotRunning_DoesNotEmitSignal)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    QSignalSpy spy(&task, &IndexTask::progressChanged);

    // Mock task as not running
    stub.set_lamda(ADDR(TaskState, isRunning), [](TaskState *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    task.onProgressChanged(50, 100);

    EXPECT_EQ(spy.count(), 0);
}

// Index Corruption Tests
TEST_F(UT_IndexTask, SetIndexCorrupted_True_SetsCorruptionFlag)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.setIndexCorrupted(true);

    EXPECT_TRUE(task.isIndexCorrupted());
}

TEST_F(UT_IndexTask, SetIndexCorrupted_False_ClearsCorruptionFlag)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    task.setIndexCorrupted(true);
    task.setIndexCorrupted(false);

    EXPECT_FALSE(task.isIndexCorrupted());
}

// Exception Handling Tests
TEST_F(UT_IndexTask, DoTask_LuceneException_SetsCorruptedFlag)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        throw Lucene::LuceneException(L"Index corrupted");
    };

    IndexTask task(IndexTask::Type::Update, "/test/path", handler);
    task.start();

    EXPECT_TRUE(task.isIndexCorrupted());
    EXPECT_EQ(task.status(), IndexTask::Status::Failed);
}

TEST_F(UT_IndexTask, DoTask_StandardException_DoesNotSetCorruptedFlag)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        throw std::runtime_error("Standard error");
    };

    IndexTask task(IndexTask::Type::Update, "/test/path", handler);
    task.start();

    EXPECT_FALSE(task.isIndexCorrupted());
    EXPECT_EQ(task.status(), IndexTask::Status::Failed);
}

TEST_F(UT_IndexTask, DoTask_UnknownException_HandledGracefully)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        throw 42;   // Unknown exception type
    };

    IndexTask task(IndexTask::Type::Update, "/test/path", handler);

    EXPECT_NO_THROW({
        task.start();
    });

    EXPECT_EQ(task.status(), IndexTask::Status::Failed);
}

TEST_F(UT_IndexTask, DoTask_HandlerReportsFatal_SetsCorruptedFlag)
{
    handlerResult.success = false;
    handlerResult.fatal = true;

    auto handler = [this](const QString &, TaskState &) -> HandlerResult {
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Update, "/test/path", handler);
    task.start();

    EXPECT_TRUE(task.isIndexCorrupted());
    EXPECT_EQ(task.status(), IndexTask::Status::Failed);
}

// Signal Emission Tests
TEST_F(UT_IndexTask, DoTask_Completion_EmitsFinishedSignal)
{
    auto handler = [this](const QString &, TaskState &) -> HandlerResult {
        return handlerResult;
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);
    QSignalSpy spy(&task, &IndexTask::finished);

    task.start();

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toInt(), static_cast<int>(IndexTask::Type::Create));
    // HandlerResult comparison would require custom operators
}

// Task State Management Tests
TEST_F(UT_IndexTask, IsRunning_PendingTask_ReturnsFalse)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/test/path", handler);

    EXPECT_FALSE(task.isRunning());
}

TEST_F(UT_IndexTask, TaskPath_ReturnsCorrectPath)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Create, "/custom/path", handler);

    EXPECT_EQ(task.taskPath(), "/custom/path");
}

TEST_F(UT_IndexTask, TaskType_ReturnsCorrectType)
{
    auto handler = [](const QString &, TaskState &) -> HandlerResult {
        return HandlerResult { true, false, false, false };
    };

    IndexTask task(IndexTask::Type::Update, "/test/path", handler);

    EXPECT_EQ(task.taskType(), IndexTask::Type::Update);
}

// No Handler Tests
TEST_F(UT_IndexTask, DoTask_NoHandler_HandledGracefully)
{
    IndexTask task(IndexTask::Type::Create, "/test/path", nullptr);

    EXPECT_NO_THROW({
        task.start();
    });

    EXPECT_EQ(task.status(), IndexTask::Status::Failed);
}
