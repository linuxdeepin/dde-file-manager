/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyub<zhangyub@uniontech.com>
 *
 * Maintainer: xinglinkun<xinglinkun@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "stubext.h"

#include "util/dde/ddesession.h"
#include "util/dde/desktopinfo.h"

#include <QApplication>

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
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(QApplication,platformName),[](){return "x11";});

    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),false);
}

TEST(DesktopInfo,wayland_dectected_wayland_session)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(QApplication,platformName),[](){return "wayland";});

    DesktopInfo info;
    EXPECT_EQ(info.waylandDectected(),true);
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
