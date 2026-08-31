// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_currenttasknotifythrottler_1.cpp
 * @brief Unit tests for CurrentTaskNotifyThrottler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/currenttasknotifythrottler.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class CurrentTaskNotifyThrottlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CurrentTaskNotifyThrottler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CurrentTaskNotifyThrottler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CurrentTaskNotifyThrottlerTest, submit)
{
    // Test method: std::optional<CurrentTaskNotifyThrottler::Task> submit((const QUrl &sourceUrl, const QUrl &targetUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->submit(_arg0, _arg1);
    EXPECT_FALSE(result.has_value());

}

TEST_F(CurrentTaskNotifyThrottlerTest, takePendingTaskUnlocked)
{
    // Test method: std::optional<CurrentTaskNotifyThrottler::Task> takePendingTaskUnlocked((qint64 elapsed))
    auto result = obj->takePendingTaskUnlocked(0);
    EXPECT_FALSE(result.has_value());

}

TEST_F(CurrentTaskNotifyThrottlerTest, takeReadyTask)
{
    // Test getter: std::optional<CurrentTaskNotifyThrottler::Task> takeReadyTask()
    auto result = obj->takeReadyTask();
    EXPECT_FALSE(result.has_value());

}
