// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendcanvasscene_1.cpp
 * @brief Unit tests for ExtendCanvasScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/extendcanvasscene.h"

#include <QTest>

using namespace ddplugin_organizer;

class ExtendCanvasSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendCanvasScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendCanvasScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendCanvasSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ExtendCanvasSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ExtendCanvasSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtendCanvasSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(ExtendCanvasSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
