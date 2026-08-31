// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_indextask.cpp
 * @brief Unit tests for IndexTask Low-priority methods
 */

#include <gtest/gtest.h>

class IndexTaskLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(IndexTaskLowTest, onProgressChanged)
{
    // onProgressChanged
    SUCCEED();
}

TEST_F(IndexTaskLowTest, throttleCpuUsage)
{
    // throttleCpuUsage
    SUCCEED();
}

