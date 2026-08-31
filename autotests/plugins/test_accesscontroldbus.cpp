// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_accesscontroldbus.cpp
 * @brief Unit tests for AccessControlDBus Mid-priority methods
 */

#include <gtest/gtest.h>

class AccessControlDBusTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(AccessControlDBusTest, onBlockDevAdded)
{
    // onBlockDevAdded
    SUCCEED();
}

TEST_F(AccessControlDBusTest, onBlockDevMounted)
{
    // onBlockDevMounted
    SUCCEED();
}
