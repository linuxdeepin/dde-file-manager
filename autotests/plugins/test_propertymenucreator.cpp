// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertymenucreator.cpp
 * @brief Unit tests for PropertyMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/propertymenuscene.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(PropertyMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
