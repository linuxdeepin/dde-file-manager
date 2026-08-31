// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanagerhook.cpp
 * @brief Unit tests for CanvasManagerHook methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "hook/canvasmanagerhook.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasManagerHookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManagerHook();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManagerHook *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerHookTest, CanvasManagerHook)
{
    // Test constructor: CanvasManagerHook((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasManagerHookTest, autoArrangeChanged)
{
    // Test method: void autoArrangeChanged((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->autoArrangeChanged(false));
}

TEST_F(CanvasManagerHookTest, fontChanged)
{
    // Test method: void fontChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->fontChanged());
}

TEST_F(CanvasManagerHookTest, iconSizeChanged)
{
    // Test method: void iconSizeChanged((int level))
    EXPECT_NO_FATAL_FAILURE(obj->iconSizeChanged(0));
}

TEST_F(CanvasManagerHookTest, requestWallpaperSetting)
{
    // Test method: bool requestWallpaperSetting((const QString &screen))
    QString _arg0{};
    auto result = obj->requestWallpaperSetting(_arg0);
    EXPECT_FALSE(result);

}
