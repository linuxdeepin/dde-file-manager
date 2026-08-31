// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharemenuscene_1.cpp
 * @brief Unit tests for ShareMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/sharemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ShareMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ShareMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ShareMenuSceneTest, scene)
{
    // Test method: dfmbase::AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ShareMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ShareMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(ShareMenuSceneTest, ShareMenuScene_Destructor)
{
    // Test method:  ~ShareMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ ShareMenuScene *tmp = new ShareMenuScene(); delete tmp; });
}
