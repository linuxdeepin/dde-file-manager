// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventhandle.cpp
 * @brief Unit tests for EventHandle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "core.h"

#include <QTest>

using namespace ddplugin_core;

class EventHandleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventHandle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventHandle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventHandleTest, devicePixelRatio)
{
    // Test getter: qreal devicePixelRatio()
    auto result = obj->devicePixelRatio();
    EXPECT_EQ(result, 0.0);

}

TEST_F(EventHandleTest, primaryScreen)
{
    // Test getter: ScreenPointer primaryScreen()
    auto result = obj->primaryScreen();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(EventHandleTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(EventHandleTest, screen)
{
    // Test method: ScreenPointer screen((const QString &name))
    QString _arg0{};
    auto result = obj->screen(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(EventHandleTest, screenSaverSetting)
{
    // Test method: bool screenSaverSetting((const QString &name))
    QString _arg0{};
    auto result = obj->screenSaverSetting(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(EventHandleTest, screens)
{
    // Test getter: QList<ScreenPointer> screens()
    auto result = obj->screens();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventHandleTest, show)
{
    // Test method: void show((QString name, int mode))
    EXPECT_NO_FATAL_FAILURE(obj->show(QString(), 0));
}

TEST_F(EventHandleTest, wallpaperSetting)
{
    // Test method: bool wallpaperSetting((const QString &name))
    QString _arg0{};
    auto result = obj->wallpaperSetting(_arg0);
    EXPECT_FALSE(result);

}
