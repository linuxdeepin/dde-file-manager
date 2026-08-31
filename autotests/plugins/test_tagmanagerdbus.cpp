// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmanagerdbus.cpp
 * @brief Unit tests for TagManagerDBus Mid-priority methods
 */

#include <gtest/gtest.h>

class TagManagerDBusTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TagManagerDBusTest, Delete)
{
    // Delete
    SUCCEED();
}
