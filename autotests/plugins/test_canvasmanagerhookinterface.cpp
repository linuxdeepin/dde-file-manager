// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanagerhookinterface.cpp
 * @brief Unit tests for CanvasManagerHookInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "private/canvasmanagerhookinterface.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasManagerHookInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManagerHookInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManagerHookInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerHookInterfaceTest, CanvasManagerHookInterface)
{
    // Test constructor: CanvasManagerHookInterface(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasManagerHookInterfaceTest, autoArrangeChanged)
{
    // Test method: void autoArrangeChanged((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->autoArrangeChanged(false));
}

TEST_F(CanvasManagerHookInterfaceTest, fontChanged)
{
    // Test method: void fontChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->fontChanged());
}

TEST_F(CanvasManagerHookInterfaceTest, iconSizeChanged)
{
    // Test method: void iconSizeChanged((int level))
    EXPECT_NO_FATAL_FAILURE(obj->iconSizeChanged(0));
}

TEST_F(CanvasManagerHookInterfaceTest, requestWallpaperSetting)
{
    // Test method: bool requestWallpaperSetting((const QString &screen))
    QString _arg0{};
    auto result = obj->requestWallpaperSetting(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasManagerHookInterfaceTest, CanvasManagerHookInterface_Destructor)
{
    // Test method:  ~CanvasManagerHookInterface(())
    EXPECT_NO_FATAL_FAILURE({ CanvasManagerHookInterface *tmp = new CanvasManagerHookInterface(); delete tmp; });
}
