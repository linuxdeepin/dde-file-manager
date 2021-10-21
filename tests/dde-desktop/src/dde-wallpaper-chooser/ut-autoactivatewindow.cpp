#include <gtest/gtest.h>
#include <stubext.h>
#include <QTest>

#include "util/dde/desktopinfo.h"

#define private public
#define protected public

#include "../dde-wallpaper-chooser/utils/autoactivatewindow_p.h"


TEST(AutoActivateWindow, test_setWatch)
{
    AutoActivateWindow act;
    ASSERT_EQ(act.d->m_watchedWidget, nullptr);
    ASSERT_FALSE(act.d->m_run);

    act.d->m_run = true;
    QWidget wid;
    act.setWatched(&wid);
    EXPECT_EQ(act.d->m_watchedWidget, nullptr);

    act.d->m_run = false;
    act.setWatched(&wid);
    EXPECT_EQ(act.d->m_watchedWidget, &wid);
}

TEST(AutoActivateWindow, test_start)
{
    stub_ext::StubExt stub;
    bool isWayland = false;
    stub.set_lamda(&DesktopInfo::waylandDectected,[&isWayland](){return isWayland;});

    bool x11 = false;
    stub.set_lamda(&AutoActivateWindowPrivate::watchOnX11,[&x11](){x11 = true;});

    bool wayland = false;
    stub.set_lamda(&AutoActivateWindowPrivate::watchOnWayland,[&wayland](){wayland = true;});

    AutoActivateWindow act;
    EXPECT_FALSE(act.start());
    EXPECT_FALSE(x11);
    EXPECT_FALSE(wayland);

    QWidget wid;
    act.d->m_watchedWidget = &wid;

    act.d->m_run = true;
    x11 = false;
    wayland = false;
    EXPECT_FALSE(act.start());
    EXPECT_FALSE(x11);
    EXPECT_FALSE(wayland);

    x11 = false;
    wayland = false;
    act.d->m_run = false;
    EXPECT_TRUE(act.start());
    EXPECT_TRUE(act.d->m_run);
    EXPECT_TRUE(x11);
    EXPECT_FALSE(wayland);

    x11 = false;
    wayland = false;
    act.d->m_run = false;
    isWayland = true;
    EXPECT_TRUE(act.start());
    EXPECT_TRUE(act.d->m_run);
    EXPECT_FALSE(x11);
    EXPECT_TRUE(wayland);
}

TEST(AutoActivateWindow, test_stop)
{
    stub_ext::StubExt stub;
    bool isWayland = false;
    stub.set_lamda(&DesktopInfo::waylandDectected,[&isWayland](){return isWayland;});

    bool x11 = false;
    stub.set_lamda(&AutoActivateWindowPrivate::watchOnX11,[&x11](){x11 = true;});

    bool wayland = false;
    stub.set_lamda(&AutoActivateWindowPrivate::watchOnWayland,[&wayland](){wayland = true;});

    AutoActivateWindow act;
    QWidget wid;
    act.d->m_watchedWidget = &wid;
    act.d->m_run = true;

    act.stop();
    EXPECT_FALSE(act.d->m_run);
    EXPECT_TRUE(x11);
    EXPECT_FALSE(wayland);

    x11 = false;
    wayland = false;
    act.d->m_run = true;
    isWayland = true;
    act.stop();
    EXPECT_FALSE(act.d->m_run);
    EXPECT_FALSE(x11);
    EXPECT_TRUE(wayland);
}

TEST(AutoActivateWindowPrivate, test_initConnect)
{
    AutoActivateWindow act;
    ASSERT_EQ(act.d->m_x11Con, nullptr);
    ASSERT_EQ(act.d->m_rootWin, 0);
    ASSERT_EQ(act.d->m_watchedWin, 0);

    EXPECT_TRUE(act.d->initConnect());
    EXPECT_NE(act.d->m_x11Con, nullptr);
    EXPECT_NE(act.d->m_rootWin, 0);
}

TEST(AutoActivateWindowPrivate, test_watchOnWayland)
{
    AutoActivateWindow act;
    QWidget wid;
    wid.winId();
    auto win = wid.windowHandle();
    if (!win)
        return;

    stub_ext::StubExt stub;
    bool ac = false;
    stub.set_lamda(&QWidget::activateWindow,[&ac](){ac = true;});

    bool isac = false;
    stub.set_lamda(&QWidget::isActiveWindow,[&isac](){return isac;});

    act.d->m_watchedWidget = &wid;
    act.d->watchOnWayland(true);
    emit win->activeChanged();
    EXPECT_TRUE(ac);
    isac = true;
    QTest::qWait(200);

    ac = false;
    act.d->watchOnWayland(false);
    emit win->activeChanged();
    EXPECT_FALSE(ac);
}

TEST(AutoActivateWindowPrivate, test_watchOnX11)
{
    AutoActivateWindow act;
    QWidget wid;
    wid.winId();
    auto win = wid.windowHandle();
    if (!win)
        return;

    stub_ext::StubExt stub;
    bool ac = false;
    stub.set_lamda(&QWidget::activateWindow,[&ac](){ac = true;});

    bool isac = false;
    stub.set_lamda(&QWidget::isActiveWindow,[&isac](){return isac;});

    act.d->m_watchedWidget = &wid;
    act.d->watchOnX11(true);
    EXPECT_NE(act.d->m_watchedWin, 0);

    emit win->activeChanged();
    EXPECT_TRUE(act.d->m_checkTimer.isActive());

    act.d->checkWindowOnX11();
    EXPECT_TRUE(ac);

    act.d->watchOnX11(false);
    EXPECT_FALSE(act.d->m_checkTimer.isActive());
}
