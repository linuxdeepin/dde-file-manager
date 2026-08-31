// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_finallyutil.cpp
 * @brief Unit tests for FinallyUtil (finallyutil.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/utils/finallyutil.h>

using namespace dfmbase;

TEST(FinallyUtilTest, ExitFuncCalledOnDestruction)
{
    bool called = false;
    {
        FinallyUtil f([&]() { called = true; });
        (void)f;
    }
    EXPECT_TRUE(called);
}

TEST(FinallyUtilTest, ExitFuncNotCalledAfterDismiss)
{
    bool called = false;
    {
        FinallyUtil f([&]() { called = true; });
        f.dismiss(true);
    }
    EXPECT_FALSE(called);
}

TEST(FinallyUtilTest, ReenableAfterDismissFalse)
{
    bool called = false;
    {
        FinallyUtil f([&]() { called = true; });
        f.dismiss(true);
        f.dismiss(false);
    }
    EXPECT_TRUE(called);
}

TEST(FinallyUtilTest, ExitFuncModifiesExternalValue)
{
    int value = 0;
    {
        FinallyUtil f([&]() { value = 42; });
        EXPECT_EQ(value, 0);
    }
    EXPECT_EQ(value, 42);
}


TEST(FinallyUtilTest, dismiss)
{
    // dismiss
    SUCCEED();
}
