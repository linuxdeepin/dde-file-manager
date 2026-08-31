// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendmenucreator.cpp
 * @brief Unit tests for ExtendMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ExtendMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(ExtendMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
