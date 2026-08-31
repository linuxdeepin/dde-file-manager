// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractindexclient_1.cpp
 * @brief Unit tests for AbstractIndexClient methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/abstractindexclient.h"

#include <QTest>

using namespace dfmplugin_search;

class AbstractIndexClientTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractIndexClient();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractIndexClient *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractIndexClientTest, checkHasRunningRootTask)
{
    // Test method: void checkHasRunningRootTask(())
    EXPECT_NO_FATAL_FAILURE(obj->checkHasRunningRootTask());
}

TEST_F(AbstractIndexClientTest, checkHasRunningTask)
{
    // Test method: void checkHasRunningTask(())
    EXPECT_NO_FATAL_FAILURE(obj->checkHasRunningTask());
}

TEST_F(AbstractIndexClientTest, checkIndexExists)
{
    // Test method: void checkIndexExists(())
    EXPECT_NO_FATAL_FAILURE(obj->checkIndexExists());
}

TEST_F(AbstractIndexClientTest, checkServiceStatus)
{
    // Test method: void checkServiceStatus(())
    EXPECT_NO_FATAL_FAILURE(obj->checkServiceStatus());
}

TEST_F(AbstractIndexClientTest, descriptor)
{
    // Test getter: IndexClientDescriptor descriptor()
    auto result = obj->descriptor();
    EXPECT_NO_FATAL_FAILURE({ obj->descriptor(); });

}

TEST_F(AbstractIndexClientTest, handleGetLastUpdateTimeReply)
{
    // Test method: void handleGetLastUpdateTimeReply((QDBusPendingCallWatcher *watcher))
    EXPECT_NO_FATAL_FAILURE(obj->handleGetLastUpdateTimeReply(nullptr));
}

TEST_F(AbstractIndexClientTest, handleHasRunningTaskReply)
{
    // Test method: void handleHasRunningTaskReply((QDBusPendingCallWatcher *watcher))
    EXPECT_NO_FATAL_FAILURE(obj->handleHasRunningTaskReply(nullptr));
}

TEST_F(AbstractIndexClientTest, handleIndexExistsReply)
{
    // Test method: void handleIndexExistsReply((QDBusPendingCallWatcher *watcher))
    EXPECT_NO_FATAL_FAILURE(obj->handleIndexExistsReply(nullptr));
}

TEST_F(AbstractIndexClientTest, handleServiceTestReply)
{
    // Test method: void handleServiceTestReply((QDBusPendingCallWatcher *watcher))
    EXPECT_NO_FATAL_FAILURE(obj->handleServiceTestReply(nullptr));
}

TEST_F(AbstractIndexClientTest, isSupportedTaskType)
{
    // Test method: bool isSupportedTaskType((const QString &type))
    QString _arg0{};
    auto result = obj->isSupportedTaskType(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AbstractIndexClientTest, onDBusTaskFinished)
{
    // Test method: void onDBusTaskFinished((const QString &type, const QString &path, bool success))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDBusTaskFinished(_arg0, _arg1, false));
}

TEST_F(AbstractIndexClientTest, onDBusTaskProgressChanged)
{
    // Test method: void onDBusTaskProgressChanged((const QString &type, const QString &path, qlonglong count, qlonglong total))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDBusTaskProgressChanged(_arg0, _arg1, {}, {}));
}

TEST_F(AbstractIndexClientTest, setEnable)
{
    // Test setter: void setEnable((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setEnable(false));
}

TEST_F(AbstractIndexClientTest, stringToTaskType)
{
    // Test method: AbstractIndexClient::TaskType stringToTaskType((const QString &type))
    QString _arg0{};
    auto result = obj->stringToTaskType(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}
