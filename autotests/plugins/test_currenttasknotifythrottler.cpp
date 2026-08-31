// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_currenttasknotifythrottler.cpp
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

TEST_F(CurrentTaskNotifyThrottlerTest, flush)
{
    // Test getter: std::optional<CurrentTaskNotifyThrottler::Task> flush()
    auto result = obj->flush();
    EXPECT_FALSE(result.has_value());

}
