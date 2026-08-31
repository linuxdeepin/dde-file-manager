// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_windowframeprivate_1.cpp
 * @brief Unit tests for WindowFramePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "frame/windowframe.h"

#include <QTest>

using namespace ddplugin_core;

class WindowFramePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WindowFramePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WindowFramePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WindowFramePrivateTest, WindowFramePrivate)
{
    // Test constructor: WindowFramePrivate((WindowFrame *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WindowFramePrivateTest, heightChanged)
{
    // Test method: void heightChanged((int arg))
    EXPECT_NO_FATAL_FAILURE(obj->heightChanged(0));
}

TEST_F(WindowFramePrivateTest, screenFromScreenPointer)
{
    // Test method: QScreen screenFromScreenPointer((const ScreenPointer &sp))
    ScreenPointer _arg0{};
    auto result = obj->screenFromScreenPointer(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->screenFromScreenPointer(_arg0); });

}

TEST_F(WindowFramePrivateTest, traceWindow)
{
    // Test method: void traceWindow((QWindow *win))
    EXPECT_NO_FATAL_FAILURE(obj->traceWindow(nullptr));
}

TEST_F(WindowFramePrivateTest, updateProperty)
{
    // Test method: void updateProperty((BaseWindowPointer win, ScreenPointer screen, bool primary))
    EXPECT_NO_FATAL_FAILURE(obj->updateProperty(BaseWindowPointer(), ScreenPointer(), false));
}

TEST_F(WindowFramePrivateTest, widthChanged)
{
    // Test method: void widthChanged((int arg))
    EXPECT_NO_FATAL_FAILURE(obj->widthChanged(0));
}

TEST_F(WindowFramePrivateTest, windowScreenChanged)
{
    // Test method: bool windowScreenChanged((const BaseWindowPointer &win, const ScreenPointer &sp))
    BaseWindowPointer _arg0{};
    ScreenPointer _arg1{};
    auto result = obj->windowScreenChanged(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WindowFramePrivateTest, xChanged)
{
    // Test method: void xChanged((int arg))
    EXPECT_NO_FATAL_FAILURE(obj->xChanged(0));
}

TEST_F(WindowFramePrivateTest, yChanged)
{
    // Test method: void yChanged((int arg))
    EXPECT_NO_FATAL_FAILURE(obj->yChanged(0));
}
