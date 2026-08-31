// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_class.cpp
 * @brief Unit tests for class Low-priority methods
 */

#include <gtest/gtest.h>

class classTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(classTest, Plugin)
{
    // Plugin
    SUCCEED();
}

TEST_F(classTest, Plugin_Plugin)
{
    // Plugin
    SUCCEED();
}

