// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_windowutils.cpp
 * @brief Unit tests for WindowUtils (windowutils.cpp)
 *
 * WindowUtils exposes platform / key-modifier helpers as static functions.
 * Under the offscreen Qt platform (no X11 / no Wayland), isX11() and
 * isWayLand() return false deterministically; keyboard-modifier helpers
 * reflect qApp state without any real keyboard. No hardware is needed.
 */

#include <gtest/gtest.h>
#include <dfm-base/utils/windowutils.h>

#include <QGuiApplication>
#include <QScreen>
#include <QTest>

using namespace dfmbase;

TEST(WindowUtilsTest, IsX11ReturnsFalseUnderOffscreenPlatform)
{
    EXPECT_EQ(QGuiApplication::platformName(), QStringLiteral("offscreen"));
    EXPECT_FALSE(WindowUtils::isX11());
}

TEST(WindowUtilsTest, IsWayLandReturnsFalseUnderOffscreenPlatform)
{
    EXPECT_EQ(QGuiApplication::platformName(), QStringLiteral("offscreen"));
    EXPECT_FALSE(WindowUtils::isWayLand());
}

TEST(WindowUtilsTest, KeyShiftIsPressedDefaultFalse)
{
    // No real keyboard input; default modifiers are NoModifier.
    EXPECT_FALSE(WindowUtils::keyShiftIsPressed());
}

TEST(WindowUtilsTest, KeyCtrlIsPressedDefaultFalse)
{
    EXPECT_FALSE(WindowUtils::keyCtrlIsPressed());
}

TEST(WindowUtilsTest, KeyAltIsPressedDefaultFalse)
{
    EXPECT_FALSE(WindowUtils::keyAltIsPressed());
}

TEST(WindowUtilsTest, CursorScreenReturnsNonNullScreen)
{
    // Under offscreen there is at least one (virtual) screen; cursorScreen()
    // falls back to the primary screen when no screen contains the cursor.
    QScreen *screen = WindowUtils::cursorScreen();
    EXPECT_NE(screen, nullptr);
}
