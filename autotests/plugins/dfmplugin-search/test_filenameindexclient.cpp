// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filenameindexclient.cpp
 * @brief Unit tests for FileNameIndexClient (filenameindexclient.cpp)
 *        — mirrors the TextIndexClient test pattern: stubs ensureInterface to
 *        prevent real D-Bus calls and exercises the inherited AbstractIndexClient
 *        public API (startTask, checkIndexExists, checkServiceStatus, etc.).
 *
 * Covers (design 9.1/9.2): client construction (singleton), task-type enum,
 * startTask for all 6 task types, async query methods (checkIndexExists/
 * checkServiceStatus/checkHasRunningTask/getLastUpdateTime), and graceful
 * handling of interface-creation failure.
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QString>
#include <QStringList>

#include "utils/filenameindexclient.h"
#include "utils/abstractindexclient.h"

#include "stubext.h"

DPSEARCH_USE_NAMESPACE

class TestFileNameIndexClient : public testing::Test
{
public:
    void SetUp() override
    {
        client = FileNameIndexClient::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    AbstractIndexClient *client = nullptr;
};

// --- Singleton ---

TEST_F(TestFileNameIndexClient, Instance_ReturnsSameInstance)
{
    auto c1 = FileNameIndexClient::instance();
    auto c2 = FileNameIndexClient::instance();
    EXPECT_NE(c1, nullptr);
    EXPECT_EQ(c1, c2);
}

TEST_F(TestFileNameIndexClient, Instance_CallableMultipleTimes)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileNameIndexClient::instance(); });
    EXPECT_NO_FATAL_FAILURE({ (void)FileNameIndexClient::instance(); });
}

// --- startTask for all task types (mocked interface) ---

TEST_F(TestFileNameIndexClient, StartTask_Create)
{
    QStringList paths = { "/home/test1", "/home/test2" };
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->startTask(FileNameIndexClient::TaskType::Create, paths); });
}

TEST_F(TestFileNameIndexClient, StartTask_Update)
{
    QStringList paths = { "/home/update" };
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->startTask(FileNameIndexClient::TaskType::Update, paths); });
}

TEST_F(TestFileNameIndexClient, StartTask_CreateFileList)
{
    QStringList paths = { "/home/file1.txt", "/home/file2.txt" };
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->startTask(FileNameIndexClient::TaskType::CreateFileList, paths); });
}

TEST_F(TestFileNameIndexClient, StartTask_UpdateFileList)
{
    QStringList paths = { "/home/updated_file.txt" };
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->startTask(FileNameIndexClient::TaskType::UpdateFileList, paths); });
}

TEST_F(TestFileNameIndexClient, StartTask_RemoveFileList)
{
    QStringList paths = { "/home/removed_file.txt" };
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->startTask(FileNameIndexClient::TaskType::RemoveFileList, paths); });
}

TEST_F(TestFileNameIndexClient, StartTask_MoveFileList)
{
    QStringList paths = { "/home/old_path.txt", "/home/new_path.txt" };
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->startTask(FileNameIndexClient::TaskType::MoveFileList, paths); });
}

// --- startTask with empty paths ---

TEST_F(TestFileNameIndexClient, StartTask_EmptyPaths_DoesNotCrash)
{
    QStringList emptyPaths;
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->startTask(FileNameIndexClient::TaskType::Create, emptyPaths); });
}

// --- Async query methods ---

TEST_F(TestFileNameIndexClient, CheckIndexExists_DoesNotCrash)
{
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->checkIndexExists(); });
}

TEST_F(TestFileNameIndexClient, CheckServiceStatus_DoesNotCrash)
{
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->checkServiceStatus(); });
}

TEST_F(TestFileNameIndexClient, CheckHasRunningTask_DoesNotCrash)
{
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->checkHasRunningTask(); });
}

TEST_F(TestFileNameIndexClient, GetLastUpdateTime_DoesNotCrash)
{
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_NO_FATAL_FAILURE({ client->getLastUpdateTime(); });
}

// --- Interface creation failure handled gracefully ---

TEST_F(TestFileNameIndexClient, EnsureInterface_FailureHandledGracefully)
{
    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;   // simulate D-Bus interface creation failure
    });
    EXPECT_NO_FATAL_FAILURE({ client->checkIndexExists(); });
    EXPECT_NO_FATAL_FAILURE({ client->checkServiceStatus(); });
    EXPECT_NO_FATAL_FAILURE({ client->getLastUpdateTime(); });
}

// --- onDBus slots (emit signals to IndexStatusController) ---

TEST_F(TestFileNameIndexClient, OnDBusTaskFinished_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE({ client->onDBusTaskFinished("Create", "/home/test", true); });
}

TEST_F(TestFileNameIndexClient, OnDBusTaskProgressChanged_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE({ client->onDBusTaskProgressChanged("Update", "/home/progress", 50, 100); });
}

// --- Enum sanity ---

TEST_F(TestFileNameIndexClient, TaskTypeEnum_AllValuesUsable)
{
    QList<FileNameIndexClient::TaskType> allTypes = {
        FileNameIndexClient::TaskType::Create,
        FileNameIndexClient::TaskType::Update,
        FileNameIndexClient::TaskType::CreateFileList,
        FileNameIndexClient::TaskType::UpdateFileList,
        FileNameIndexClient::TaskType::RemoveFileList,
        FileNameIndexClient::TaskType::MoveFileList
    };

    stub.set_lamda(&AbstractIndexClient::ensureInterface, [](AbstractIndexClient *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QStringList testPaths = { "/test/path" };
    for (auto type : allTypes) {
        EXPECT_NO_FATAL_FAILURE({ client->startTask(type, testPaths); });
    }
}

TEST_F(TestFileNameIndexClient, ServiceStatusEnum_AllValuesExist)
{
    QList<FileNameIndexClient::ServiceStatus> allStatuses = {
        FileNameIndexClient::ServiceStatus::Available,
        FileNameIndexClient::ServiceStatus::Unavailable,
        FileNameIndexClient::ServiceStatus::Error
    };
    for (auto status : allStatuses) {
        (void)status;   // just verify the enum values exist and are usable
    }
    SUCCEED();
}
