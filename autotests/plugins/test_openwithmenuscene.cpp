// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithmenuscene.cpp
 * @brief Unit tests for OpenWithMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/openwithmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OpenWithMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithMenuSceneTest, OpenWithMenuScene)
{
    // Test constructor: OpenWithMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpenWithMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpenWithMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpenWithMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpenWithMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(OpenWithMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpenWithMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
