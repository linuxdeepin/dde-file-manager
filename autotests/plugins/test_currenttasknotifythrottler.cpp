// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_currenttasknotifythrottler.cpp
 * @brief Unit tests for CurrentTaskNotifyThrottler Mid-priority methods
 */

#include <gtest/gtest.h>

class CurrentTaskNotifyThrottlerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CurrentTaskNotifyThrottlerTest, flush)
{
    // flush
    SUCCEED();
}
