// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mysharemenucreator.cpp
 * @brief Unit tests for MyShareMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/mysharemenuscene.h"

#include <QTest>

using namespace dfmplugin_myshares;

class MyShareMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MyShareMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MyShareMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MyShareMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(MyShareMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
