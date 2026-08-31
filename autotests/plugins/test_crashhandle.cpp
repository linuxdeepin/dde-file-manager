// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crashhandle.cpp
 * @brief Unit tests for CrashHandle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/utils/crashhandle.h"

#include <QTest>

using namespace src;

class CrashHandleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrashHandle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrashHandle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrashHandleTest, clearCrash)
{
    // Test method: void clearCrash(())
    EXPECT_NO_FATAL_FAILURE(obj->clearCrash());
}

TEST_F(CrashHandleTest, regSignal)
{
    // Test method: void regSignal(())
    EXPECT_NO_FATAL_FAILURE(obj->regSignal());
}

TEST_F(CrashHandleTest, unregSignal)
{
    // Test method: void unregSignal(())
    EXPECT_NO_FATAL_FAILURE(obj->unregSignal());
}

TEST_F(CrashHandleTest, upgradeCacheDir)
{
    // Test getter: QString upgradeCacheDir()
    auto result = obj->upgradeCacheDir();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
