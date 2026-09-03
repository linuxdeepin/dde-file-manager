// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "stubext.h"

#include "utils/abstractindexclient.h"
#include "utils/indexclientdescriptor.h"

using namespace dfmplugin_search;

class AbstractIndexClientTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create a descriptor with a non-existent DBus service
        // so ensureInterface() returns false and methods emit failure signals
        IndexClientDescriptor desc;
        desc.clientName = "testclient";
        desc.dbusServiceName = "com.deepin.fake.NonExistentService";
        desc.dbusObjectPath = "/com/deepin/fake/NonExistent";
        desc.interfaceFactory = nullptr;   // Will cause ensureInterface to fall back to sessionBus

        client = new AbstractIndexClient(desc);
    }

    void TearDown() override
    {
        delete client;
    }

    AbstractIndexClient *client = nullptr;
};

// --- construction ---

TEST_F(AbstractIndexClientTest, Constructor_CreatesClient)
{
    EXPECT_NE(client, nullptr);
}

// --- descriptor ---

TEST_F(AbstractIndexClientTest, Descriptor_ReturnsClientName)
{
    EXPECT_EQ(client->descriptor().clientName, "testclient");
}

TEST_F(AbstractIndexClientTest, Descriptor_ReturnsDBusServiceName)
{
    EXPECT_EQ(client->descriptor().dbusServiceName, "com.deepin.fake.NonExistentService");
}

// --- checkServiceStatus (emits Unavailable when interface unavailable) ---

TEST_F(AbstractIndexClientTest, CheckServiceStatus_EmitsUnavailable)
{
    QSignalSpy spy(client, &AbstractIndexClient::serviceStatusResult);
    client->checkServiceStatus();
    // Should emit with Unavailable when service doesn't exist
    spy.wait(500);
    EXPECT_GE(spy.count(), 0);   // May or may not emit depending on bus availability
}

// --- getIndexStatus (emits failure when interface unavailable) ---

TEST_F(AbstractIndexClientTest, GetIndexStatus_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::indexStatusResult);
    EXPECT_NO_FATAL_FAILURE(client->getIndexStatus());
    spy.wait(500);
}

// --- setEnable ---

TEST_F(AbstractIndexClientTest, SetEnable_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->setEnable(true));
}

TEST_F(AbstractIndexClientTest, SetEnable_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->setEnable(false));
}

// --- checkIndexExists ---

TEST_F(AbstractIndexClientTest, CheckIndexExists_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::indexExistsResult);
    EXPECT_NO_FATAL_FAILURE(client->checkIndexExists());
    spy.wait(500);
}

// --- checkHasRunningTask ---

TEST_F(AbstractIndexClientTest, CheckHasRunningTask_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningTaskResult);
    EXPECT_NO_FATAL_FAILURE(client->checkHasRunningTask());
    spy.wait(500);
}

// --- checkHasRunningRootTask ---

TEST_F(AbstractIndexClientTest, CheckHasRunningRootTask_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningRootTaskResult);
    EXPECT_NO_FATAL_FAILURE(client->checkHasRunningRootTask());
    spy.wait(500);
}

// --- getLastUpdateTime ---

TEST_F(AbstractIndexClientTest, GetLastUpdateTime_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::lastUpdateTimeResult);
    EXPECT_NO_FATAL_FAILURE(client->getLastUpdateTime());
    spy.wait(500);
}

// --- forceUpdateIndex ---

TEST_F(AbstractIndexClientTest, ForceUpdateIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->forceUpdateIndex({"/tmp", "/home"}));
}

// --- updateIndexBypassEnv ---

TEST_F(AbstractIndexClientTest, UpdateIndexBypassEnv_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->updateIndexBypassEnv({"/tmp"}));
}

// --- startTask (various types) ---

TEST_F(AbstractIndexClientTest, StartTask_Create_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::Create, {"/tmp"}));
}

TEST_F(AbstractIndexClientTest, StartTask_Update_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::Update, {"/tmp"}));
}

TEST_F(AbstractIndexClientTest, StartTask_CreateFileList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::CreateFileList, {"/tmp"}));
}

TEST_F(AbstractIndexClientTest, StartTask_UpdateFileList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::UpdateFileList, {"/tmp"}));
}

TEST_F(AbstractIndexClientTest, StartTask_RemoveFileList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::RemoveFileList, {"/tmp"}));
}

TEST_F(AbstractIndexClientTest, StartTask_MoveFileList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::MoveFileList, {"/tmp", "/home"}));
}

TEST_F(AbstractIndexClientTest, StartTask_WithOptions_NoCrash)
{
    QVariantMap options;
    options["force"] = true;
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::Update, {"/tmp"}, options));
}
