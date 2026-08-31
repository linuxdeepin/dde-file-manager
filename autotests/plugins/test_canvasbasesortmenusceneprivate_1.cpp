// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasbasesortmenusceneprivate_1.cpp
 * @brief Unit tests for CanvasBaseSortMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/canvasbasesortmenuscene.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasBaseSortMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasBaseSortMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasBaseSortMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasBaseSortMenuScenePrivateTest, CanvasBaseSortMenuScenePrivate)
{
    // Test constructor: CanvasBaseSortMenuScenePrivate((CanvasBaseSortMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasBaseSortMenuScenePrivateTest, sendToRule)
{
    // Test getter: QStringList sendToRule()
    auto result = obj->sendToRule();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasBaseSortMenuScenePrivateTest, stageToRule)
{
    // Test getter: QStringList stageToRule()
    auto result = obj->stageToRule();
    EXPECT_TRUE(result.isEmpty());

}
