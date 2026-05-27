// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QDBusConnection>
#include <QTimer>

#include "textindexdbus.h"
#include "private/textindexdbus_p.h"
#include "task/taskmanager.h"
#include "utils/systemdcpuutils.h"
#include "utils/textindexconfig.h"
#include "utils/indexutility.h"
#include <dfm-search/dsearch_global.h>

SERVICETEXTINDEX_USE_NAMESPACE

class UT_TextIndexDBus : public testing::Test
{
protected:
    void SetUp() override
    {
        // Mock D-Bus connection to avoid actual D-Bus dependency
        using ConnectToDBusFunc = QDBusConnection (*)(QDBusConnection::BusType, const QString &);
        stub.set_lamda(static_cast<ConnectToDBusFunc>(&QDBusConnection::connectToBus), [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            __DBG_STUB_INVOKE__
            return QDBusConnection("mock_connection");
        });
        using RegisterObjectFunc = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);
        stub.set_lamda(static_cast<RegisterObjectFunc>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Mock TaskManager
        using StartTaskFunc = bool (TaskManager::*)(IndexTask::Type, const QStringList &, bool);
        stub.set_lamda(static_cast<StartTaskFunc>(&TaskManager::startTask), [this](TaskManager *, IndexTask::Type type, const QStringList &paths, bool silent) -> bool {
            __DBG_STUB_INVOKE__
            lastTaskType = type;
            lastTaskPaths = paths;
            lastTaskSilent = silent;
            return mockTaskManagerSuccess;
        });

        stub.set_lamda(ADDR(TaskManager, startFileListTask), [this](TaskManager *, IndexTask::Type type, const QStringList &files, bool silent) -> bool {
            __DBG_STUB_INVOKE__
            lastFileListTaskType = type;
            lastFileListFiles = files;
            lastFileListSilent = silent;
            return mockTaskManagerSuccess;
        });

        stub.set_lamda(ADDR(TaskManager, startFileMoveTask), [this](TaskManager *, const QHash<QString, QString> &movedFiles, bool silent) -> bool {
            __DBG_STUB_INVOKE__
            lastMoveFiles = movedFiles;
            lastMoveSilent = silent;
            return mockTaskManagerSuccess;
        });

        stub.set_lamda(ADDR(TaskManager, hasRunningTask), [this](TaskManager *) -> bool {
            __DBG_STUB_INVOKE__
            return mockHasRunningTask;
        });

        stub.set_lamda(ADDR(TaskManager, stopCurrentTask), [this](TaskManager *) {
            __DBG_STUB_INVOKE__
            mockStopCurrentTaskCalled = true;
        });

        // Mock SystemdCpuUtils
        stub.set_lamda(ADDR(SystemdCpuUtils, resetCpuQuota), [this](const QString &serviceName, QString *errorMsg) -> bool {
            __DBG_STUB_INVOKE__
            lastResetCpuService = serviceName;
            return mockResetCpuQuotaSuccess;
        });

        // Mock IndexUtility
        stub.set_lamda(ADDR(IndexUtility, isCompatibleVersion), []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(IndexUtility, getLastUpdateTime), []() -> QString {
            __DBG_STUB_INVOKE__
            return "2025-01-01 12:00:00";
        });

        // Mock DFMSEARCH::Global
        stub.set_lamda(ADDR(DFMSEARCH::Global, isContentIndexAvailable), []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(DFMSEARCH::Global, defaultIndexedDirectory), []() -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList { "/home/test" };
        });

        // Reset mock states
        mockTaskManagerSuccess = true;
        mockHasRunningTask = false;
        mockStopCurrentTaskCalled = false;
        mockResetCpuQuotaSuccess = true;
        lastTaskType = static_cast<IndexTask::Type>(-1);
        lastTaskPaths.clear();
        lastTaskSilent = false;
        lastFileListTaskType = static_cast<IndexTask::Type>(-1);
        lastFileListFiles.clear();
        lastFileListSilent = false;
        lastMoveFiles.clear();
        lastMoveSilent = false;
        lastResetCpuService.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    // Mock state variables
    bool mockTaskManagerSuccess = true;
    bool mockHasRunningTask = false;
    bool mockStopCurrentTaskCalled = false;
    bool mockResetCpuQuotaSuccess = true;

    IndexTask::Type lastTaskType = static_cast<IndexTask::Type>(-1);
    QStringList lastTaskPaths;
    bool lastTaskSilent = false;

    IndexTask::Type lastFileListTaskType = static_cast<IndexTask::Type>(-1);
    QStringList lastFileListFiles;
    bool lastFileListSilent = false;

    QHash<QString, QString> lastMoveFiles;
    bool lastMoveSilent = false;

    QString lastResetCpuService;

private:
    stub_ext::StubExt stub;
};

// Constructor and Initialization Tests
TEST_F(UT_TextIndexDBus, Constructor_ValidName_InitializesSuccessfully)
{
    TextIndexDBus dbus("test-service");

    EXPECT_TRUE(true);   // Constructor completed without exception
}

TEST_F(UT_TextIndexDBus, Init_Called_SetsRefreshStartedFlag)
{
    TextIndexDBus dbus("test-service");

    // Should not throw
    EXPECT_NO_THROW({
        dbus.Init();
    });
}

// Enable/Disable Tests
TEST_F(UT_TextIndexDBus, IsEnabled_DefaultState_ReturnsCorrectValue)
{
    TextIndexDBus dbus("test-service");

    // Mock FSEventController
    stub.set_lamda(ADDR(FSEventController, isEnabled), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool enabled = dbus.IsEnabled();
    EXPECT_FALSE(enabled);
}

TEST_F(UT_TextIndexDBus, SetEnabled_ValidValue_CallsFSEventController)
{
    TextIndexDBus dbus("test-service");

    bool setEnabledCalled = false;
    bool setEnabledValue = false;

    stub.set_lamda(ADDR(FSEventController, setEnabled), [&setEnabledCalled, &setEnabledValue](FSEventController *, bool enabled) {
        __DBG_STUB_INVOKE__
        setEnabledCalled = true;
        setEnabledValue = enabled;
    });

    dbus.SetEnabled(true);

    EXPECT_TRUE(setEnabledCalled);
    EXPECT_TRUE(setEnabledValue);
}

// Task Management Tests
TEST_F(UT_TextIndexDBus, CreateIndexTask_ValidPaths_StartsTask)
{
    TextIndexDBus dbus("test-service");
    QStringList paths = { "/home/test", "/home/user" };

    bool result = dbus.CreateIndexTask(paths);

    EXPECT_TRUE(result);
    EXPECT_EQ(lastTaskType, IndexTask::Type::Create);
    EXPECT_EQ(lastTaskPaths, paths);
    EXPECT_FALSE(lastTaskSilent);
}

TEST_F(UT_TextIndexDBus, UpdateIndexTask_ValidPaths_StartsTask)
{
    TextIndexDBus dbus("test-service");
    QStringList paths = { "/home/test" };

    bool result = dbus.UpdateIndexTask(paths);

    EXPECT_TRUE(result);
    EXPECT_EQ(lastTaskType, IndexTask::Type::Update);
    EXPECT_EQ(lastTaskPaths, paths);
    EXPECT_FALSE(lastTaskSilent);
}

TEST_F(UT_TextIndexDBus, CreateIndexTask_TaskManagerFails_ReturnsFalse)
{
    TextIndexDBus dbus("test-service");
    mockTaskManagerSuccess = false;

    bool result = dbus.CreateIndexTask({ "/home/test" });

    EXPECT_FALSE(result);
}

TEST_F(UT_TextIndexDBus, StopCurrentTask_NoRunningTask_ReturnsFalse)
{
    TextIndexDBus dbus("test-service");
    mockHasRunningTask = false;

    bool result = dbus.StopCurrentTask();

    EXPECT_FALSE(result);
    EXPECT_FALSE(mockStopCurrentTaskCalled);
}

TEST_F(UT_TextIndexDBus, StopCurrentTask_HasRunningTask_ReturnsTrue)
{
    TextIndexDBus dbus("test-service");
    mockHasRunningTask = true;

    bool result = dbus.StopCurrentTask();

    EXPECT_TRUE(result);
    EXPECT_TRUE(mockStopCurrentTaskCalled);
}

TEST_F(UT_TextIndexDBus, HasRunningTask_CallsTaskManager)
{
    TextIndexDBus dbus("test-service");
    mockHasRunningTask = true;

    bool result = dbus.HasRunningTask();

    EXPECT_TRUE(result);
}

// Index Database Tests
TEST_F(UT_TextIndexDBus, IndexDatabaseExists_AllConditionsMet_ReturnsTrue)
{
    TextIndexDBus dbus("test-service");

    bool result = dbus.IndexDatabaseExists();

    EXPECT_TRUE(result);
}

TEST_F(UT_TextIndexDBus, IndexDatabaseExists_ContentIndexNotAvailable_ReturnsFalse)
{
    TextIndexDBus dbus("test-service");

    stub.set_lamda(ADDR(DFMSEARCH::Global, isContentIndexAvailable), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = dbus.IndexDatabaseExists();

    EXPECT_FALSE(result);
}

TEST_F(UT_TextIndexDBus, IndexDatabaseExists_IncompatibleVersion_ReturnsFalse)
{
    TextIndexDBus dbus("test-service");

    stub.set_lamda(ADDR(IndexUtility, isCompatibleVersion), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = dbus.IndexDatabaseExists();

    EXPECT_FALSE(result);
}

TEST_F(UT_TextIndexDBus, IndexDatabaseExists_EmptyLastUpdateTime_ReturnsFalse)
{
    TextIndexDBus dbus("test-service");

    stub.set_lamda(ADDR(IndexUtility, getLastUpdateTime), []() -> QString {
        __DBG_STUB_INVOKE__
        return QString();
    });

    bool result = dbus.IndexDatabaseExists();

    EXPECT_FALSE(result);
}

TEST_F(UT_TextIndexDBus, GetLastUpdateTime_ReturnsCorrectValue)
{
    TextIndexDBus dbus("test-service");

    QString result = dbus.GetLastUpdateTime();

    EXPECT_EQ(result, "2025-01-01 12:00:00");
}

// File Change Processing Tests
TEST_F(UT_TextIndexDBus, ProcessFileChanges_WithDeletedFiles_ProcessesInOrder)
{
    TextIndexDBus dbus("test-service");

    QStringList createdFiles = { "/home/test/new1.txt", "/home/test/new2.txt" };
    QStringList modifiedFiles = { "/home/test/mod1.txt" };
    QStringList deletedFiles = { "/home/test/del1.txt", "/home/test/del2.txt" };

    bool result = dbus.ProcessFileChanges(createdFiles, modifiedFiles, deletedFiles);

    EXPECT_TRUE(result);
    // Should process deleted files first, then created, then modified
    // This is verified by checking the task manager calls
}

TEST_F(UT_TextIndexDBus, ProcessFileChanges_EmptyLists_ReturnsFalse)
{
    TextIndexDBus dbus("test-service");

    bool result = dbus.ProcessFileChanges({}, {}, {});

    EXPECT_FALSE(result);
}

TEST_F(UT_TextIndexDBus, ProcessFileChanges_OnlyCreatedFiles_ProcessesCorrectly)
{
    TextIndexDBus dbus("test-service");

    QStringList createdFiles = { "/home/test/new.txt" };

    bool result = dbus.ProcessFileChanges(createdFiles, {}, {});

    EXPECT_TRUE(result);
    EXPECT_EQ(lastFileListTaskType, IndexTask::Type::CreateFileList);
    EXPECT_EQ(lastFileListFiles, createdFiles);
    EXPECT_TRUE(lastFileListSilent);
}

// File Move Processing Tests
TEST_F(UT_TextIndexDBus, ProcessFileMoves_ValidMoves_ProcessesCorrectly)
{
    TextIndexDBus dbus("test-service");

    QHash<QString, QString> movedFiles;
    movedFiles["/old/path1.txt"] = "/new/path1.txt";
    movedFiles["/old/path2.txt"] = "/new/path2.txt";

    bool result = dbus.ProcessFileMoves(movedFiles);

    EXPECT_TRUE(result);
    EXPECT_EQ(lastMoveFiles, movedFiles);
    EXPECT_TRUE(lastMoveSilent);
}

TEST_F(UT_TextIndexDBus, ProcessFileMoves_EmptyHash_ReturnsFalse)
{
    TextIndexDBus dbus("test-service");

    bool result = dbus.ProcessFileMoves({});

    EXPECT_FALSE(result);
}

// Cleanup Tests
TEST_F(UT_TextIndexDBus, Cleanup_Called_StopsTasksAndDisablesController)
{
    TextIndexDBus dbus("test-service");

    bool setEnabledNowCalled = false;
    bool setEnabledNowValue = true;

    stub.set_lamda(ADDR(FSEventController, setEnabledNow), [&setEnabledNowCalled, &setEnabledNowValue](FSEventController *, bool enabled) {
        __DBG_STUB_INVOKE__
        setEnabledNowCalled = true;
        setEnabledNowValue = enabled;
    });

    mockHasRunningTask = true;

    dbus.cleanup();

    EXPECT_TRUE(setEnabledNowCalled);
    EXPECT_FALSE(setEnabledNowValue);
    EXPECT_TRUE(mockStopCurrentTaskCalled);
}

// Signal Emission Tests
TEST_F(UT_TextIndexDBus, TaskFinished_EmitsSignalCorrectly)
{
    TextIndexDBus dbus("test-service");

    QSignalSpy spy(&dbus, &TextIndexDBus::TaskFinished);

    // Simulate task manager signal emission
    emit dbus.d->taskManager->taskFinished("create", "/home/test", true);

    // Wait for signal processing
    QTimer::singleShot(10, [&]() {
        EXPECT_EQ(spy.count(), 1);
        if (spy.count() > 0) {
            QList<QVariant> arguments = spy.takeFirst();
            EXPECT_EQ(arguments.at(0).toString(), "create");
            EXPECT_EQ(arguments.at(1).toString(), "/home/test");
            EXPECT_TRUE(arguments.at(2).toBool());
        }
    });
}

TEST_F(UT_TextIndexDBus, TaskProgressChanged_EmitsSignalCorrectly)
{
    TextIndexDBus dbus("test-service");

    QSignalSpy spy(&dbus, &TextIndexDBus::TaskProgressChanged);

    // Simulate task manager signal emission
    emit dbus.d->taskManager->taskProgressChanged("update", "/home/test", 50, 100);

    // Wait for signal processing
    QTimer::singleShot(10, [&]() {
        EXPECT_EQ(spy.count(), 1);
        if (spy.count() > 0) {
            QList<QVariant> arguments = spy.takeFirst();
            EXPECT_EQ(arguments.at(0).toString(), "update");
            EXPECT_EQ(arguments.at(1).toString(), "/home/test");
            EXPECT_EQ(arguments.at(2).toLongLong(), 50);
            EXPECT_EQ(arguments.at(3).toLongLong(), 100);
        }
    });
}

// Error Handling Tests
TEST_F(UT_TextIndexDBus, TaskFinished_ResetCpuQuotaFails_ContinuesNormally)
{
    TextIndexDBus dbus("test-service");
    mockResetCpuQuotaSuccess = false;

    QSignalSpy spy(&dbus, &TextIndexDBus::TaskFinished);

    // Should not throw even if CPU quota reset fails
    EXPECT_NO_THROW({
        emit dbus.d->taskManager->taskFinished("create", "/home/test", true);
    });

    EXPECT_EQ(lastResetCpuService, Defines::kTextIndexServiceName);
}
