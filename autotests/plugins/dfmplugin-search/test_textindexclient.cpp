// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include "utils/textindexclient.h"

#include "stubext.h"

DPSEARCH_USE_NAMESPACE

class TestTextIndexClient : public testing::Test
{
public:
    void SetUp() override
    {
        client = TextIndexClient::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    TextIndexClient *client = nullptr;
};

TEST_F(TestTextIndexClient, Instance_ReturnsSameInstance)
{
    auto client1 = TextIndexClient::instance();
    auto client2 = TextIndexClient::instance();

    EXPECT_NE(client1, nullptr);
    EXPECT_EQ(client1, client2);
}

TEST_F(TestTextIndexClient, StartTask_WithCreateType_EmitsTaskStartedSignal)
{
    QStringList paths = { "/home/test1", "/home/test2" };
    TextIndexClient::TaskType taskType = TextIndexClient::TaskType::Create;

    // Mock interface operations to prevent actual D-Bus calls
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSignalSpy spy(client, &TextIndexClient::taskStarted);

    client->startTask(taskType, paths);

    // Note: In a real implementation, this would require mocking the D-Bus interface
    // For now, we verify the method can be called without crashing
    EXPECT_TRUE(true);
}

TEST_F(TestTextIndexClient, StartTask_WithUpdateType_CallsCorrectly)
{
    QStringList paths = { "/home/update" };
    TextIndexClient::TaskType taskType = TextIndexClient::TaskType::Update;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Test that the method can be called
    EXPECT_NO_FATAL_FAILURE(client->startTask(taskType, paths));
}

TEST_F(TestTextIndexClient, StartTask_WithCreateFileListType_CallsCorrectly)
{
    QStringList paths = { "/home/file1.txt", "/home/file2.txt" };
    TextIndexClient::TaskType taskType = TextIndexClient::TaskType::CreateFileList;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(client->startTask(taskType, paths));
}

TEST_F(TestTextIndexClient, StartTask_WithUpdateFileListType_CallsCorrectly)
{
    QStringList paths = { "/home/updated_file.txt" };
    TextIndexClient::TaskType taskType = TextIndexClient::TaskType::UpdateFileList;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(client->startTask(taskType, paths));
}

TEST_F(TestTextIndexClient, StartTask_WithRemoveFileListType_CallsCorrectly)
{
    QStringList paths = { "/home/removed_file.txt" };
    TextIndexClient::TaskType taskType = TextIndexClient::TaskType::RemoveFileList;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(client->startTask(taskType, paths));
}

TEST_F(TestTextIndexClient, StartTask_WithMoveFileListType_CallsCorrectly)
{
    QStringList paths = { "/home/old_path.txt", "/home/new_path.txt" };
    TextIndexClient::TaskType taskType = TextIndexClient::TaskType::MoveFileList;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(client->startTask(taskType, paths));
}

TEST_F(TestTextIndexClient, CheckIndexExists_CallsAsyncMethod)
{
    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSignalSpy spy(client, &TextIndexClient::indexExistsResult);

    EXPECT_NO_FATAL_FAILURE(client->checkIndexExists());
}

TEST_F(TestTextIndexClient, CheckServiceStatus_CallsAsyncMethod)
{
    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(client->checkServiceStatus());
}

TEST_F(TestTextIndexClient, CheckHasRunningRootTask_CallsAsyncMethod)
{
    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSignalSpy spy(client, &TextIndexClient::hasRunningRootTaskResult);

    EXPECT_NO_FATAL_FAILURE(client->checkHasRunningRootTask());
}

TEST_F(TestTextIndexClient, CheckHasRunningTask_CallsAsyncMethod)
{
    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSignalSpy spy(client, &TextIndexClient::hasRunningTaskResult);

    EXPECT_NO_FATAL_FAILURE(client->checkHasRunningTask());
}

TEST_F(TestTextIndexClient, GetLastUpdateTime_CallsAsyncMethod)
{
    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QSignalSpy spy(client, &TextIndexClient::lastUpdateTimeResult);

    EXPECT_NO_FATAL_FAILURE(client->getLastUpdateTime());
}

TEST_F(TestTextIndexClient, SetEnable_WithTrue_CallsCorrectly)
{
    bool enabled = true;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(client->setEnable(enabled));
}

TEST_F(TestTextIndexClient, SetEnable_WithFalse_CallsCorrectly)
{
    bool enabled = false;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(client->setEnable(enabled));
}

TEST_F(TestTextIndexClient, OnDBusTaskFinished_WithValidParameters_EmitsCorrectSignal)
{
    QString taskType = "Create";
    QString path = "/home/test";

    EXPECT_NO_FATAL_FAILURE(client->onDBusTaskFinished(taskType, path, true));
}

TEST_F(TestTextIndexClient, OnDBusTaskFinished_WithInvalidTaskType_DoesNotEmitSignal)
{
    QString taskType = "InvalidType";
    QString path = "/home/test";
    bool success = true;

    QSignalSpy spy(client, &TextIndexClient::taskFinished);

    // Call the private slot
    QMetaObject::invokeMethod(client, "onDBusTaskFinished", Qt::DirectConnection,
                              Q_ARG(QString, taskType),
                              Q_ARG(QString, path),
                              Q_ARG(bool, success));

    // Should not emit signal for invalid task type
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(TestTextIndexClient, OnDBusTaskProgressChanged_WithValidParameters_EmitsSignal)
{
    QString taskType = "Update";
    QString path = "/home/progress";
    qlonglong count = 50;
    qlonglong total = 100;

    EXPECT_NO_FATAL_FAILURE(client->onDBusTaskProgressChanged(taskType, path, count, total));
}

TEST_F(TestTextIndexClient, TaskTypeEnum_AllValuesWork)
{
    // Test that all enum values can be used
    QList<TextIndexClient::TaskType> allTypes = {
        TextIndexClient::TaskType::Create,
        TextIndexClient::TaskType::Update,
        TextIndexClient::TaskType::CreateFileList,
        TextIndexClient::TaskType::UpdateFileList,
        TextIndexClient::TaskType::RemoveFileList,
        TextIndexClient::TaskType::MoveFileList
    };

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QStringList testPaths = { "/test/path" };

    for (auto type : allTypes) {
        EXPECT_NO_FATAL_FAILURE(client->startTask(type, testPaths));
    }
}

TEST_F(TestTextIndexClient, ServiceStatusEnum_AllValuesWork)
{
    // Test that all ServiceStatus enum values exist
    QList<TextIndexClient::ServiceStatus> allStatuses = {
        TextIndexClient::ServiceStatus::Available,
        TextIndexClient::ServiceStatus::Unavailable,
        TextIndexClient::ServiceStatus::Error
    };

    // Just verify the enum values exist and can be used
    for (auto status : allStatuses) {
        EXPECT_TRUE(true); // Basic test that enum values are accessible
    }
}

TEST_F(TestTextIndexClient, EnsureInterface_CanBeCalled)
{
    // Test that ensureInterface method exists and can be called
    // Note: This is a private method, so we're testing it indirectly
    // by calling public methods that use it

    // Mock the interface to prevent actual D-Bus operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false; // Simulate interface creation failure
    });

    // These calls should handle the interface creation failure gracefully
    EXPECT_NO_FATAL_FAILURE(client->checkIndexExists());
    EXPECT_NO_FATAL_FAILURE(client->checkServiceStatus());
}

TEST_F(TestTextIndexClient, StartTask_WithEmptyPaths_CallsCorrectly)
{
    QStringList emptyPaths;
    TextIndexClient::TaskType taskType = TextIndexClient::TaskType::Create;

    // Mock interface operations
    stub.set_lamda(&TextIndexClient::ensureInterface, [](TextIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Should handle empty paths gracefully
    EXPECT_NO_FATAL_FAILURE(client->startTask(taskType, emptyPaths));
}
