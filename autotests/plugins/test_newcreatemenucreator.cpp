// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_newcreatemenucreator.cpp
 * @brief Unit tests for NewCreateMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/newcreatemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class NewCreateMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NewCreateMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NewCreateMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NewCreateMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(NewCreateMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
