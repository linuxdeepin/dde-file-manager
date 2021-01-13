#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "util/dde/ddesession.h"
#include "util/dde/desktopinfo.h"

TEST(DdeSession,register_null)
{
    const char *envName = "DDE_SESSION_PROCESS_COOKIE_ID";
    qunsetenv(envName);
    Dde::Session::RegisterDdeSession();
    EXPECT_EQ(true,qgetenv(envName).isEmpty());
}

TEST(DdeSession,register_empty)
{
    const char *envName = "DDE_SESSION_PROCESS_COOKIE_ID";
    qputenv(envName,"");
    Dde::Session::RegisterDdeSession();
    EXPECT_EQ(true,qgetenv(envName).isEmpty());
}

TEST(DdeSession,register_content)
{
    const char *envName = "DDE_SESSION_PROCESS_COOKIE_ID";
    qputenv(envName,"test_app_desktop");
    Dde::Session::RegisterDdeSession();
    EXPECT_EQ(true,qgetenv(envName).isEmpty());
}

TEST(DesktopInfo,wayland_dectected_nowayland)
{
    qunsetenv("XDG_SESSION_TYPE");
    qunsetenv("WAYLAND_DISPLAY");

    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),false);
}

TEST(DesktopInfo,wayland_dectected_wayland_session)
{
    qunsetenv("XDG_SESSION_TYPE");
    qunsetenv("WAYLAND_DISPLAY");

    qputenv("XDG_SESSION_TYPE","wayland");
    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),true);
    qunsetenv("XDG_SESSION_TYPE");
}

TEST(DesktopInfo,wayland_dectected_x11_session)
{
    qunsetenv("XDG_SESSION_TYPE");
    qunsetenv("WAYLAND_DISPLAY");

    qputenv("XDG_SESSION_TYPE","x11");
    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),false);
    qunsetenv("XDG_SESSION_TYPE");
}

TEST(DesktopInfo,wayland_dectected_wayland_display)
{
    qputenv("WAYLAND_DISPLAY","wayland");
    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),true);
    qunsetenv("WAYLAND_DISPLAY");
}

TEST(DesktopInfo,wayland_dectected_xcb_display)
{
    qputenv("WAYLAND_DISPLAY","xcb");
    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),false);
    qunsetenv("WAYLAND_DISPLAY");
}

TEST(DesktopInfo,wayland_dectected_wl_xcb)
{
    qputenv("XDG_SESSION_TYPE","wayland");
    qputenv("WAYLAND_DISPLAY","xcb");
    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),true);
    qunsetenv("WAYLAND_DISPLAY");
    qunsetenv("XDG_SESSION_TYPE");
}

TEST(WindowManager,xdg_null_desktop_null)
{
    qunsetenv("XDG_CURRENT_DESKTOP");
    qunsetenv("GNOME_DESKTOP_SESSION_ID");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::OTHER,info.windowManager());
}

TEST(WindowManager,xdg_empty_desktop_empty)
{
    qputenv("XDG_CURRENT_DESKTOP","");
    qputenv("GNOME_DESKTOP_SESSION_ID","");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::OTHER,info.windowManager());
}

TEST(WindowManager,xdg_wl_desktop_emptys)
{
    qputenv("XDG_CURRENT_DESKTOP","wayland");
    qputenv("GNOME_DESKTOP_SESSION_ID","");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::OTHER,info.windowManager());
}

TEST(WindowManager,xdg_wl_desktop_wl)
{
    qputenv("XDG_CURRENT_DESKTOP","wayland");
    qputenv("GNOME_DESKTOP_SESSION_ID","wayland");
    DesktopInfo info;
    EXPECT_NE(DesktopInfo::WM::OTHER,info.windowManager());
}


TEST(WindowManager,desktop_gnome1)
{
    qputenv("XDG_CURRENT_DESKTOP","GNOME1");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::GNOME,info.windowManager());
}

TEST(WindowManager,desktop_gnome)
{
    qputenv("XDG_CURRENT_DESKTOP","GNOME");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::GNOME,info.windowManager());
}

TEST(WindowManager,xdg_kde_desktop_gnome)
{
    qputenv("XDG_CURRENT_DESKTOP","KDE");
    qputenv("GNOME_DESKTOP_SESSION_ID","gnome");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::GNOME,info.windowManager());
}

TEST(WindowManager,xdg_kde_desktop_empty)
{
    qputenv("XDG_CURRENT_DESKTOP","KDE");
    qputenv("GNOME_DESKTOP_SESSION_ID","");
    DesktopInfo info;
    EXPECT_NE(DesktopInfo::WM::GNOME,info.windowManager());
}

TEST(WindowManager,xdg_null_desktop_wayland)
{
    qunsetenv("XDG_CURRENT_DESKTOP");
    qputenv("GNOME_DESKTOP_SESSION_ID","wayland");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::GNOME,info.windowManager());
}


TEST(WindowManager,kde_kde_desktop_null)
{
    qunsetenv("XDG_CURRENT_DESKTOP");
    qunsetenv("GNOME_DESKTOP_SESSION_ID");
    qputenv("KDE_FULL_SESSION","kde");
    qunsetenv("DESKTOP_SESSION");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::KDE,info.windowManager());
}


TEST(WindowManager,kde_empty_desktop_kdeplasma)
{
    qunsetenv("XDG_CURRENT_DESKTOP");
    qunsetenv("GNOME_DESKTOP_SESSION_ID");
    qputenv("KDE_FULL_SESSION","");
    qputenv("DESKTOP_SESSION","kde-plasma");
    DesktopInfo info;
    EXPECT_EQ(DesktopInfo::WM::KDE,info.windowManager());
}

TEST(WindowManager,kde_empty_desktop_kde)
{
    qunsetenv("XDG_CURRENT_DESKTOP");
    qunsetenv("GNOME_DESKTOP_SESSION_ID");
    qputenv("KDE_FULL_SESSION","");
    qputenv("DESKTOP_SESSION","kde");
    DesktopInfo info;
    EXPECT_NE(DesktopInfo::WM::KDE,info.windowManager());
}
