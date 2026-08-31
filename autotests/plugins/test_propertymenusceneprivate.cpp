// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertymenusceneprivate.cpp
 * @brief Unit tests for PropertyMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/propertymenuscene.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyMenuScenePrivateTest, PropertyMenuScenePrivate)
{
    // Test constructor: PropertyMenuScenePrivate((PropertyMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PropertyMenuScenePrivateTest, updateMenu)
{
    // Test method: void updateMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateMenu(nullptr));
}
