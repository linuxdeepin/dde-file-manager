// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screenproxyqt.cpp
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

TEST_F(ScreenProxyQtTest, checkUsedScreens)
{
    // Test bool getter: checkUsedScreens()
    bool result = obj->checkUsedScreens();
    EXPECT_FALSE(result);

}

TEST_F(ScreenProxyQtTest, devicePixelRatio)
{
    // Test getter: qreal devicePixelRatio()
    auto result = obj->devicePixelRatio();
    EXPECT_EQ(result, 0.0);

}

TEST_F(ScreenProxyQtTest, logicScreens)
{
    // Test getter: QList<ScreenPointer> logicScreens()
    auto result = obj->logicScreens();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ScreenProxyQtTest, onScreenRemoved)
{
    // Test method: void onScreenRemoved((QScreen *screen))
    EXPECT_NO_FATAL_FAILURE(obj->onScreenRemoved(nullptr));
}

TEST_F(ScreenProxyQtTest, primaryScreen)
{
    // Test getter: ScreenPointer primaryScreen()
    auto result = obj->primaryScreen();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(ScreenProxyQtTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(ScreenProxyQtTest, screen)
{
    // Test method: ScreenPointer screen((const QString &name))
    QString _arg0{};
    auto result = obj->screen(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(ScreenProxyQtTest, ScreenProxyQt)
{
    // Test constructor: ScreenProxyQt((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
