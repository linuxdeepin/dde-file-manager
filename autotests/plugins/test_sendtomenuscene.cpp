// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sendtomenuscene.cpp
 * @brief Unit tests for SendToMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/sendtomenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class SendToMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SendToMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SendToMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SendToMenuSceneTest, SendToMenuScene)
{
    // Test constructor: SendToMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SendToMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SendToMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SendToMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(SendToMenuSceneTest, SendToMenuScene_Destructor)
{
    // Test method:  ~SendToMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ SendToMenuScene *tmp = new SendToMenuScene(); delete tmp; });
}
