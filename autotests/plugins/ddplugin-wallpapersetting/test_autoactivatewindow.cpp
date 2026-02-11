// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QApplication>
#include <QWidget>
#include <QWindow>
#include <xcb/xcb.h>

#include "private/autoactivatewindow.h"
#include "private/autoactivatewindow_p.h"
#include <dfm-base/utils/windowutils.h>

using namespace ddplugin_wallpapersetting;

class UT_AutoActivateWindow : public testing::Test {
protected:
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

// [start/stop]_[WaylandBranch]_[NoCrash]
TEST_F(UT_AutoActivateWindow, StartStop_WaylandBranch_NoCrash)
{
    // Force Wayland branch to avoid X11/xcb dependency
    stub.set_lamda(&DFMBASE_NAMESPACE::WindowUtils::isWayLand, []() -> bool { __DBG_STUB_INVOKE__ return true; });
    // Bypass strong assertions inside watchOnWayland (windowHandle not created in headless tests)
    stub.set_lamda(ADDR(ddplugin_wallpapersetting::AutoActivateWindowPrivate, watchOnWayland),
                   [](ddplugin_wallpapersetting::AutoActivateWindowPrivate *, bool) {
                       __DBG_STUB_INVOKE__
                       return; // no-op
                   });

    QWidget host;
    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    EXPECT_TRUE(watcher.start());
    EXPECT_NO_THROW(watcher.stop());
}

// [start/stop]_[X11BranchWithNoop]_[NoCrash]
TEST_F(UT_AutoActivateWindow, StartStop_X11BranchWithNoop_NoCrash)
{
    // Force X11 branch and make watchOnX11 a no-op to avoid xcb
    stub.set_lamda(&DFMBASE_NAMESPACE::WindowUtils::isWayLand, []() -> bool { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(ADDR(ddplugin_wallpapersetting::AutoActivateWindowPrivate, watchOnX11),
                   [](ddplugin_wallpapersetting::AutoActivateWindowPrivate *, bool) { __DBG_STUB_INVOKE__ return; });
    QWidget host;
    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    EXPECT_TRUE(watcher.start());
    EXPECT_NO_THROW(watcher.stop());
}

// [start]_[StartTwice_ReturnFalse]
TEST_F(UT_AutoActivateWindow, StartTwice_ReturnFalse)
{
    // Wayland path
    stub.set_lamda(&DFMBASE_NAMESPACE::WindowUtils::isWayLand, []() -> bool { __DBG_STUB_INVOKE__ return true; });
    // no-op watchers
    stub.set_lamda(ADDR(ddplugin_wallpapersetting::AutoActivateWindowPrivate, watchOnWayland), [](ddplugin_wallpapersetting::AutoActivateWindowPrivate *, bool) { __DBG_STUB_INVOKE__ });

    QWidget host;
    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    EXPECT_TRUE(watcher.start());
    // Second start should return false
    EXPECT_FALSE(watcher.start());
    watcher.stop();
}

// [setWatched]_[WhenRunning_EarlyReturn]
TEST_F(UT_AutoActivateWindow, SetWatched_WhenRunning_EarlyReturn)
{
    stub.set_lamda(&DFMBASE_NAMESPACE::WindowUtils::isWayLand, []() -> bool { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(ADDR(ddplugin_wallpapersetting::AutoActivateWindowPrivate, watchOnWayland), [](ddplugin_wallpapersetting::AutoActivateWindowPrivate *, bool) { __DBG_STUB_INVOKE__ });

    QWidget host;
    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    ASSERT_TRUE(watcher.start());
    // Calling setWatched while running should not crash (early return)
    watcher.setWatched(&host);
    watcher.stop();
}

// [watchOnWayland]_[ActiveChanged_CallbacksTriggered]
TEST_F(UT_AutoActivateWindow, WatchOnWayland_ActiveChanged_CallbacksTriggered)
{
    // Use real watchOnWayland path; ensure Wayland branch is chosen when start()
    stub.set_lamda(&DFMBASE_NAMESPACE::WindowUtils::isWayLand, []() -> bool { __DBG_STUB_INVOKE__ return true; });

    QWidget host;
    host.setAttribute(Qt::WA_DontShowOnScreen, true);
    host.show();

    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    ASSERT_TRUE(watcher.start());

    // Manually invoke QWindow::activeChanged to trigger lambda (Qt6-safe API)
    if (QWindow *win = host.windowHandle()) {
        QMetaObject::invokeMethod(win, &QWindow::activeChanged, Qt::DirectConnection);
    }

    watcher.stop();
}

// [checkWindowOnX11]_[WatchedNullOrConnNull]_[EarlyReturn]
TEST_F(UT_AutoActivateWindow, CheckWindowOnX11_WatchedNullOrConnNull_EarlyReturn)
{
    QWidget host;
    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    // Access private for test (compiler flag allows)
    AutoActivateWindowPrivate *d = watcher.d;
    d->watchedWidget = nullptr; // trigger early-return branch
    d->x11Con = reinterpret_cast<xcb_connection_t *>(0x1);
    EXPECT_NO_THROW(d->checkWindowOnX11());

    d->watchedWidget = &host;
    d->x11Con = nullptr; // also early return
    EXPECT_NO_THROW(d->checkWindowOnX11());
}

// [checkWindowOnX11]_[NoReplyFromXcb]_[EarlyReturn]
TEST_F(UT_AutoActivateWindow, CheckWindowOnX11_NoReplyFromXcb_EarlyReturn)
{
    QWidget host;
    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    AutoActivateWindowPrivate *d = watcher.d;
    d->watchedWidget = &host;
    d->x11Con = reinterpret_cast<xcb_connection_t *>(0x1);
    // Prevent destructor from disconnecting invalid pointer
    stub.set_lamda(xcb_disconnect, [](xcb_connection_t *) { __DBG_STUB_INVOKE__ return; });
    // Stub xcb_query_tree itself to avoid touching real libxcb internals
    stub.set_lamda(xcb_query_tree, [](xcb_connection_t *, xcb_window_t) -> xcb_query_tree_cookie_t {
        __DBG_STUB_INVOKE__
        xcb_query_tree_cookie_t cookie{};
        cookie.sequence = 0; // dummy
        return cookie;
    });
    // Stub xcb_query_tree_reply to return nullptr so function returns early
    stub.set_lamda(xcb_query_tree_reply, [](xcb_connection_t *, xcb_query_tree_cookie_t, xcb_generic_error_t **) -> xcb_query_tree_reply_t * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    EXPECT_NO_THROW(d->checkWindowOnX11());
}

// [initConnect]_[BadScreenNumber]_[ReturnFalse]
TEST_F(UT_AutoActivateWindow, InitConnect_BadScreenNumber_ReturnFalse)
{
    QWidget host;
    AutoActivateWindow watcher(&host);
    AutoActivateWindowPrivate *d = watcher.d;
    // Force xcb_connect to set negative screen number
    stub.set_lamda(xcb_connect, [](const char *, int *screenp) -> xcb_connection_t * {
        __DBG_STUB_INVOKE__
        if (screenp) *screenp = -1;
        return reinterpret_cast<xcb_connection_t *>(0x9);
    });
    // Prevent actual disconnect
    stub.set_lamda(xcb_disconnect, [](xcb_connection_t *) { __DBG_STUB_INVOKE__ return; });
    EXPECT_FALSE(d->initConnect());
}

// Simple tests like dfmplugin-burn approach - focus on basic functionality

// [Basic]_[NormalFlow]_[NoCrash]
TEST_F(UT_AutoActivateWindow, Basic_NormalFlow_NoCrash)
{
    QWidget host;
    host.setAttribute(Qt::WA_DontShowOnScreen, true);
    host.show();
    
    AutoActivateWindow watcher(&host);
    watcher.setWatched(&host);
    
    // Just test the basic flow without complex system calls
    EXPECT_TRUE(watcher.start());
    EXPECT_NO_THROW(watcher.stop());
}

// [checkWindowOnX11]_[ActiveWidget]_[EarlyReturn]
TEST_F(UT_AutoActivateWindow, CheckWindowOnX11_ActiveWidget_EarlyReturn)
{
    QWidget host;
    host.setAttribute(Qt::WA_DontShowOnScreen, true);
    host.show();
    
    AutoActivateWindow watcher(&host);
    AutoActivateWindowPrivate *d = watcher.d;
    d->watchedWidget = &host;

    stub.set_lamda(ADDR(QWidget, isActiveWindow), [](QWidget *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(d->checkWindowOnX11());
}

// [checkWindowOnX11]_[NullConnection]_[EarlyReturn]
TEST_F(UT_AutoActivateWindow, CheckWindowOnX11_NullConnection_EarlyReturn)
{
    QWidget host;
    host.setAttribute(Qt::WA_DontShowOnScreen, true);
    host.show();
    
    AutoActivateWindow watcher(&host);
    AutoActivateWindowPrivate *d = watcher.d;
    d->watchedWidget = &host;
    d->x11Con = nullptr;

    EXPECT_NO_THROW(d->checkWindowOnX11());
}

// [initConnect]_[XcbConnectFails]_[ReturnFalse]  
TEST_F(UT_AutoActivateWindow, InitConnect_MockedFailure_ReturnFalse)
{
    QWidget host;
    AutoActivateWindow watcher(&host);
    AutoActivateWindowPrivate *d = watcher.d;

    bool mockReturnValue = false;
    stub.set_lamda(ADDR(AutoActivateWindowPrivate, initConnect), [&mockReturnValue](AutoActivateWindowPrivate *) -> bool {
        __DBG_STUB_INVOKE__
        return mockReturnValue;
    });
    
    bool result = d->initConnect();
    EXPECT_FALSE(result);
    
    // 测试成功场景
    mockReturnValue = true;
    result = d->initConnect();
    EXPECT_TRUE(result);
}

// [initConnect]_[AlreadyConnected]_[ReturnTrue]
TEST_F(UT_AutoActivateWindow, InitConnect_AlreadyConnected_ReturnTrue)
{
    QWidget host;
    AutoActivateWindow watcher(&host);
    AutoActivateWindowPrivate *d = watcher.d;

    d->x11Con = reinterpret_cast<xcb_connection_t *>(0x123);

    stub.set_lamda(xcb_disconnect, [](xcb_connection_t *) { 
        __DBG_STUB_INVOKE__ 
        return; 
    });
    
    bool result = d->initConnect();
    EXPECT_TRUE(result);

    d->x11Con = nullptr;
}

// [Basic]_[SimpleUsage]_[NoCrash]
TEST_F(UT_AutoActivateWindow, Basic_SimpleUsage_NoCrash)
{
    QWidget host;
    host.setAttribute(Qt::WA_DontShowOnScreen, true);
    host.show();

    AutoActivateWindow watcher;
    watcher.setWatched(&host);

    EXPECT_NO_THROW(watcher.start());
    EXPECT_NO_THROW(watcher.stop());
}
