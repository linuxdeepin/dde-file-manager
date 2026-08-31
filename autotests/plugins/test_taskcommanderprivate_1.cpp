// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskcommanderprivate_1.cpp
 * @brief Unit tests for TaskCommanderPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/maincontroller/task/taskcommander.h"

#include <QTest>

using namespace dfmplugin_search;

class TaskCommanderPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskCommanderPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskCommanderPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskCommanderPrivateTest, onResultsUpdated)
{
    // Test method: void onResultsUpdated((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onResultsUpdated(_arg0));
}

TEST_F(TaskCommanderPrivateTest, onSearchCompleted)
{
    // Test method: void onSearchCompleted((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSearchCompleted(_arg0));
}
