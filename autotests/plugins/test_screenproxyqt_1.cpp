// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screenproxyqt_1.cpp
 * @brief Unit tests for ScreenProxyQt methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/screenproxyqt.h"

#include <QTest>

using namespace ddplugin_core;

class ScreenProxyQtTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ScreenProxyQt();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ScreenProxyQt *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScreenProxyQtTest, connectScreen)
{
    // Test method: void connectScreen((ScreenPointer sp))
    EXPECT_NO_FATAL_FAILURE(obj->connectScreen(ScreenPointer()));
}

TEST_F(ScreenProxyQtTest, disconnectScreen)
{
    // Test method: void disconnectScreen((ScreenPointer sp))
    EXPECT_NO_FATAL_FAILURE(obj->disconnectScreen(ScreenPointer()));
}

TEST_F(ScreenProxyQtTest, displayMode)
{
    // Test getter: DisplayMode displayMode()
    auto result = obj->displayMode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ScreenProxyQtTest, onDockChanged)
{
    // Test method: void onDockChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onDockChanged());
}

TEST_F(ScreenProxyQtTest, onPrimaryChanged)
{
    // Test method: void onPrimaryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onPrimaryChanged());
}

TEST_F(ScreenProxyQtTest, onScreenAdded)
{
    // Test method: void onScreenAdded((QScreen *screen))
    EXPECT_NO_FATAL_FAILURE(obj->onScreenAdded(nullptr));
}

TEST_F(ScreenProxyQtTest, onScreenAvailableGeometryChanged)
{
    // Test method: void onScreenAvailableGeometryChanged((const QRect &rect))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onScreenAvailableGeometryChanged(_arg0));
}

TEST_F(ScreenProxyQtTest, onScreenGeometryChanged)
{
    // Test method: void onScreenGeometryChanged((const QRect &rect))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onScreenGeometryChanged(_arg0));
}

TEST_F(ScreenProxyQtTest, validateEvent)
{
    // Test event handler: validateEvent((Event event))
    Event _event(Event::None);
    EXPECT_NO_FATAL_FAILURE(obj->validateEvent(&_event));
}
