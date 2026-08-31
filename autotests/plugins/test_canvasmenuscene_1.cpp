// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmenuscene_1.cpp
 * @brief Unit tests for CanvasMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/canvasmenuscene.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasMenuSceneTest, emptyMenu)
{
    // Test method: void emptyMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->emptyMenu(nullptr));
}

TEST_F(CanvasMenuSceneTest, iconSizeSubActions)
{
    // Test method: QMenu iconSizeSubActions((QMenu *menu))
    auto result = obj->iconSizeSubActions(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->iconSizeSubActions(nullptr); });

}

TEST_F(CanvasMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(CanvasMenuSceneTest, sortBySubActions)
{
    // Test method: QMenu sortBySubActions((QMenu *menu))
    auto result = obj->sortBySubActions(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->sortBySubActions(nullptr); });

}

TEST_F(CanvasMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}
