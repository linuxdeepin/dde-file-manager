// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_basewindow.cpp
 * @brief Unit tests for BaseWindow Low-priority methods
 */

#include <gtest/gtest.h>

class BaseWindowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BaseWindowTest, init)
{
    // init
    SUCCEED();
}

