// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskcommander.cpp
 * @brief Unit tests for TaskCommander methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/maincontroller/task/taskcommander.h"

#include <QTest>

using namespace dfmplugin_search;

class TaskCommanderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskCommander();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskCommander *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskCommanderTest, TaskCommander)
{
    // Test constructor: TaskCommander((QString taskId, const QUrl &url, const QString &keyword, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TaskCommanderTest, getResults)
{
    // Test getter: DFMSearchResultMap getResults()
    auto result = obj->getResults();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TaskCommanderTest, getResultsUrls)
{
    // Test getter: QList<QUrl> getResultsUrls()
    auto result = obj->getResultsUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TaskCommanderTest, onWorkThreadFinished)
{
    // Test method: void onWorkThreadFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onWorkThreadFinished());
}

TEST_F(TaskCommanderTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(TaskCommanderTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(TaskCommanderTest, taskID)
{
    // Test getter: QString taskID()
    auto result = obj->taskID();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TaskCommanderTest, TaskCommander_Destructor)
{
    // Test method:  ~TaskCommander(())
    EXPECT_NO_FATAL_FAILURE({ TaskCommander *tmp = new TaskCommander(); delete tmp; });
}
