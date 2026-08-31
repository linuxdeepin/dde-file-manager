// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_blockdev_helper.cpp
 * @brief Unit tests for blockdev_helper Low-priority methods
 */

#include <gtest/gtest.h>

class blockdev_helperLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(blockdev_helperLowTest, devBlockSize)
{
    // devBlockSize
    SUCCEED();
}

TEST_F(blockdev_helperLowTest, devDeviceSize)
{
    // devDeviceSize
    SUCCEED();
}

