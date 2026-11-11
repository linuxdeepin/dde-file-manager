// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ddeshellwindow.cpp
 * @brief Unit tests for the dde shell skip-dock-preview helper
 *
 * The helper only acts when the process runs on the Wayland platform, so the
 * offscreen/xcb test platform exercises the no-Wayland guard: every call must
 * be a clean no-op returning false, and a null window must be rejected before
 * anything else is touched. On a Wayland platform the request is accepted
 * (and may stay pending when the compositor does not expose the protocol),
 * which the tests mirror through a platform check. The protocol path itself
 * runs on Treeland and is verified manually.
 */

#include <gtest/gtest.h>

#include <dfm-base/utils/private/ddeshellwindow_p.h>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QWindow>

using namespace dfmbase::ddeshell;

namespace {

bool runsOnWaylandPlatform()
{
    return QGuiApplication::platformName() == QStringLiteral("wayland");
}

}   // namespace

TEST(DDEShellWindowTest, NullWindowIsRejected)
{
    // Before anything else: no window, no result.
    EXPECT_FALSE(setSkipDockPreview(nullptr, true));
    EXPECT_FALSE(setSkipDockPreview(nullptr, false));
}

TEST(DDEShellWindowTest, WithoutWaylandPlatformEveryCallIsANoOp)
{
    // The offscreen/xcb platform has no Wayland native interface, so the
    // helper must fail cleanly instead of opening a second connection or
    // touching the window handle. Only on Wayland is the request accepted.
    QWindow window;
    EXPECT_EQ(setSkipDockPreview(&window, true), runsOnWaylandPlatform());
    EXPECT_EQ(setSkipDockPreview(&window, false), runsOnWaylandPlatform());
}

TEST(DDEShellWindowTest, ShownWindowKeepsTheGuardOrder)
{
    // With a live platform surface the platform guard must still fire first:
    // no crash and no side effects on non-Wayland platforms, regardless of
    // how often the bit is toggled.
    QWindow window;
    window.show();
    QCoreApplication::processEvents();

    const bool expected = runsOnWaylandPlatform();
    EXPECT_EQ(setSkipDockPreview(&window, true), expected);
    EXPECT_EQ(setSkipDockPreview(&window, false), expected);
    EXPECT_EQ(setSkipDockPreview(&window, true), expected);
}

TEST(DDEShellWindowTest, RepeatedCallsDoNotCrash)
{
    QWindow window;
    window.show();
    QCoreApplication::processEvents();

    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_THROW(setSkipDockPreview(&window, true));
        EXPECT_NO_THROW(setSkipDockPreview(&window, false));
    }
}

TEST(DDEShellWindowTest, WindowsAreHandledIndependently)
{
    // The per-window bookkeeping must not confuse one window with another:
    // closing the first must not take the second's request down with it.
    const bool expected = runsOnWaylandPlatform();
    {
        QWindow first;
        QWindow second;
        first.show();
        second.show();
        QCoreApplication::processEvents();

        EXPECT_EQ(setSkipDockPreview(&first, true), expected);
        EXPECT_EQ(setSkipDockPreview(&second, true), expected);
        // Destroying one window must not invalidate the other's request.
        first.close();
        QCoreApplication::processEvents();
        EXPECT_EQ(setSkipDockPreview(&second, true), expected);
    }
}
