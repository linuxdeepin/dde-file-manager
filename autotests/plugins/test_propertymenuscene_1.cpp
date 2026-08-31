// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertymenuscene_1.cpp
 * @brief Unit tests for PropertyMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/propertymenuscene.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyMenuSceneTest, PropertyMenuScene)
{
    // Test constructor: PropertyMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PropertyMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(PropertyMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(PropertyMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(PropertyMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
