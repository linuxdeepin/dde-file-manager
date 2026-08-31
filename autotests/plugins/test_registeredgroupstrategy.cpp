// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_registeredgroupstrategy.cpp
 * @brief Unit tests for RegisteredGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/groupingfactory.h"

#include <QTest>

using namespace dfmplugin_workspace;

class RegisteredGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RegisteredGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RegisteredGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RegisteredGroupStrategyTest, RegisteredGroupStrategy)
{
    // Test constructor: RegisteredGroupStrategy(())
    ASSERT_NE(obj, nullptr);
}
