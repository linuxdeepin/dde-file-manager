// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionlibmenuscene_1.cpp
 * @brief Unit tests for ExtensionLibMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/extensionlibmenuscene.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionLibMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionLibMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionLibMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionLibMenuSceneTest, ExtensionLibMenuScene)
{
    // Test constructor: ExtensionLibMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtensionLibMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ExtensionLibMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(ExtensionLibMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ExtensionLibMenuSceneTest, ExtensionLibMenuScene_Destructor)
{
    // Test method:  ~ExtensionLibMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ ExtensionLibMenuScene *tmp = new ExtensionLibMenuScene(); delete tmp; });
}
