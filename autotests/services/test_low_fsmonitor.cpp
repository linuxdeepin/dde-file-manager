// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_fsmonitor.cpp
 * @brief Unit tests for FSMonitor Low-priority methods
 */

#include <gtest/gtest.h>

class FSMonitorLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FSMonitorLowTest, initialize)
{
    // initialize
    SUCCEED();
}

