// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screenproxydbus_1.cpp
 * @brief Unit tests for ScreenProxyDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/screenproxydbus.h"

#include <QTest>

using namespace ddplugin_core;

class ScreenProxyDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ScreenProxyDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ScreenProxyDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScreenProxyDBusTest, ScreenProxyDBus)
{
    // Test constructor: ScreenProxyDBus((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ScreenProxyDBusTest, connectScreen)
{
    // Test method: void connectScreen((ScreenPointer sp))
    EXPECT_NO_FATAL_FAILURE(obj->connectScreen(ScreenPointer()));
}

TEST_F(ScreenProxyDBusTest, devicePixelRatio)
{
    // Test getter: qreal devicePixelRatio()
    auto result = obj->devicePixelRatio();
    EXPECT_EQ(result, 0.0);

}

TEST_F(ScreenProxyDBusTest, disconnectScreen)
{
    // Test method: void disconnectScreen((ScreenPointer sp))
    EXPECT_NO_FATAL_FAILURE(obj->disconnectScreen(ScreenPointer()));
}

TEST_F(ScreenProxyDBusTest, displayMode)
{
    // Test getter: DisplayMode displayMode()
    auto result = obj->displayMode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ScreenProxyDBusTest, onDockChanged)
{
    // Test method: void onDockChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onDockChanged());
}

TEST_F(ScreenProxyDBusTest, onModeChanged)
{
    // Test method: void onModeChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onModeChanged());
}

TEST_F(ScreenProxyDBusTest, onScreenGeometryChanged)
{
    // Test method: void onScreenGeometryChanged((const QRect &rect))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onScreenGeometryChanged(_arg0));
}

TEST_F(ScreenProxyDBusTest, primaryScreen)
{
    // Test getter: ScreenPointer primaryScreen()
    auto result = obj->primaryScreen();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(ScreenProxyDBusTest, processEvent)
{
    // Test method: void processEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->processEvent());
}

TEST_F(ScreenProxyDBusTest, screen)
{
    // Test method: ScreenPointer screen((const QString &name))
    QString _arg0{};
    auto result = obj->screen(_arg0);
    EXPECT_NE(result.get(), nullptr);

}
