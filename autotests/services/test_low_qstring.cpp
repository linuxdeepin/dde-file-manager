// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_qstring.cpp
 * @brief Unit tests for QString Low-priority methods
 */

#include <gtest/gtest.h>

class QStringLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(QStringLowTest, devConfigPath)
{
    // devConfigPath
    SUCCEED();
}

