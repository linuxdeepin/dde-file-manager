// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

class UT_Basic : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(UT_Basic, BasicTest)
{
    // Basic functionality test - placeholder for plugin unit tests
    EXPECT_TRUE(true);
} 