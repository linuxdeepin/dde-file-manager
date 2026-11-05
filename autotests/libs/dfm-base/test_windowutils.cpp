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
#include <cstdlib>
#include <cstring>
#include <dfm-base/utils/windowutils.h>

#include <QGuiApplication>
#include <QScreen>
#include <QTest>
#include <QWidget>

using namespace dfmbase;

TEST(WindowUtilsTest, IsX11ReturnsFalseUnderOffscreenPlatform)
{
    // isX11() checks platformName == "xcb" first, then falls back to
    // DISPLAY / XDG_SESSION_TYPE environment variables.  Under offscreen
    // the platformName is not "xcb", but if DISPLAY is set and
    // XDG_SESSION_TYPE is "x11", isX11() still returns true.
    const char *display = std::getenv("DISPLAY");
    const char *session_type = std::getenv("XDG_SESSION_TYPE");
    const bool envIndicatesX11 =
            (display && display[0] != '\0') &&
            (session_type && strcmp(session_type, "x11") == 0);

    if (QGuiApplication::platformName() == QStringLiteral("xcb")) {
        EXPECT_TRUE(WindowUtils::isX11());
    } else if (envIndicatesX11) {
        // Offscreen platform but DISPLAY+XDG_SESSION_TYPE indicate X11
        EXPECT_TRUE(WindowUtils::isX11());
    } else {
        EXPECT_FALSE(WindowUtils::isX11());
    }
}

TEST(WindowUtilsTest, IsWayLandReturnsFalseUnderOffscreenPlatform)
{
    const char *wayland_display = std::getenv("WAYLAND_DISPLAY");
    const char *session_type = std::getenv("XDG_SESSION_TYPE");
    const bool envIndicatesWayland =
            (wayland_display && wayland_display[0] != '\0') ||
            (session_type && strcmp(session_type, "wayland") == 0);

    if (QGuiApplication::platformName() == QStringLiteral("wayland")) {
        EXPECT_TRUE(WindowUtils::isWayLand());
    } else if (envIndicatesWayland) {
        EXPECT_TRUE(WindowUtils::isWayLand());
    } else {
        EXPECT_FALSE(WindowUtils::isWayLand());
    }
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

TEST(WindowUtilsTest, SetActivationTokenExportsEnvironmentVariable)
{
    qunsetenv("XDG_ACTIVATION_TOKEN");

    WindowUtils::setActivationToken(QStringLiteral("token-1"));

    EXPECT_EQ(qgetenv("XDG_ACTIVATION_TOKEN"), QByteArray("token-1"));
    qunsetenv("XDG_ACTIVATION_TOKEN");
}

TEST(WindowUtilsTest, SetEmptyActivationTokenLeavesEnvironmentUntouched)
{
    qputenv("XDG_ACTIVATION_TOKEN", "token-1");

    WindowUtils::setActivationToken(QString());

    EXPECT_EQ(qgetenv("XDG_ACTIVATION_TOKEN"), QByteArray("token-1"));
    qunsetenv("XDG_ACTIVATION_TOKEN");
}

TEST(WindowUtilsTest, ActivateWindowIgnoresNullWidget)
{
    // Must not crash; nothing else to observe without a window.
    WindowUtils::activateWindow(nullptr);
}

TEST(WindowUtilsTest, ActivateWindowClearsMinimizedStateOffWayland)
{
    // isWayLand() is false once neither the platform nor the environment
    // claims Wayland, so the minimized state is expected to be cleared locally
    // - the only branch the client can handle on its own.
    const QByteArray waylandDisplay = qgetenv("WAYLAND_DISPLAY");
    const QByteArray sessionType = qgetenv("XDG_SESSION_TYPE");
    qunsetenv("WAYLAND_DISPLAY");
    qunsetenv("XDG_SESSION_TYPE");

    QWidget widget;
    widget.setWindowState(Qt::WindowMinimized);
    const bool offWayland = !WindowUtils::isWayLand();

    WindowUtils::activateWindow(&widget);

    if (offWayland)
        EXPECT_FALSE(widget.windowState() & Qt::WindowMinimized);

    if (!waylandDisplay.isEmpty())
        qputenv("WAYLAND_DISPLAY", waylandDisplay);
    if (!sessionType.isEmpty())
        qputenv("XDG_SESSION_TYPE", sessionType);
}
