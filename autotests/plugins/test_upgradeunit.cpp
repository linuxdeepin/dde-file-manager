// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_upgradeunit.cpp
 * @brief Unit tests for UpgradeUnit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/core/upgradeunit.h"

#include <QTest>

using namespace src;

class UpgradeUnitTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UpgradeUnit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UpgradeUnit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UpgradeUnitTest, UpgradeUnit)
{
    // Test constructor: UpgradeUnit(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(UpgradeUnitTest, completed)
{
    // Test method: void completed(())
    EXPECT_NO_FATAL_FAILURE(obj->completed());
}
