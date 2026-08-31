// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmenusceneprivate.cpp
 * @brief Unit tests for CanvasMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/canvasmenuscene.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasMenuScenePrivateTest, CanvasMenuScenePrivate)
{
    // Test constructor: CanvasMenuScenePrivate((CanvasMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasMenuScenePrivateTest, checkOrganizerPlugin)
{
    // Test bool getter: checkOrganizerPlugin()
    bool result = obj->checkOrganizerPlugin();
    EXPECT_FALSE(result);

}
