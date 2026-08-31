// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_windowframe_1.cpp
 * @brief Unit tests for WindowFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "frame/windowframe.h"

#include <QTest>

using namespace ddplugin_core;

class WindowFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WindowFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WindowFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WindowFrameTest, WindowFrame)
{
    // Test constructor: WindowFrame((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WindowFrameTest, bindedScreens)
{
    // Test getter: QStringList bindedScreens()
    auto result = obj->bindedScreens();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WindowFrameTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(WindowFrameTest, onAvailableGeometryChanged)
{
    // Test method: void onAvailableGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onAvailableGeometryChanged());
}

TEST_F(WindowFrameTest, onGeometryChanged)
{
    // Test method: void onGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onGeometryChanged());
}

TEST_F(WindowFrameTest, WindowFrame_Destructor)
{
    // Test method:  ~WindowFrame(())
    EXPECT_NO_FATAL_FAILURE({ WindowFrame *tmp = new WindowFrame(); delete tmp; });
}
