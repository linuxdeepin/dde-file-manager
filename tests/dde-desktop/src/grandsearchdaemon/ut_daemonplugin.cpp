// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemonplugin.h"
#include "daemonlibrary.h"

#include "stub.h"
#include "stubext.h"

#include <gtest/gtest.h>

using namespace GrandSearch;

TEST(DaemonPlugin, initialize_load)
{
    DaemonPlugin plugin;
    stub_ext::StubExt stub;

    bool load = false;;
    stub.set_lamda(&DaemonLibrary::load, [&load](){
        load = true;
        return true;
    });

    EXPECT_TRUE(plugin.initialize());
    EXPECT_TRUE(load);
    EXPECT_NE(plugin.library, nullptr);
}

TEST(DaemonPlugin, initialize_load_false)
{
    DaemonPlugin plugin;
    stub_ext::StubExt stub;

    bool load = false;;
    stub.set_lamda(&DaemonLibrary::load, [&load](){
        load = true;
        return false;
    });

    EXPECT_FALSE(plugin.initialize());
    EXPECT_TRUE(load);
    ASSERT_EQ(plugin.library, nullptr);
}

TEST(DaemonPlugin, initialize_checkpath)
{
    DaemonPlugin plugin;
    stub_ext::StubExt stub;
    stub.set_lamda(&DaemonLibrary::load,[](){
        return true;
    });

    plugin.initialize();
    ASSERT_NE(plugin.library, nullptr);

    EXPECT_TRUE(plugin.library->libPath.startsWith("/usr/lib/"));
    EXPECT_TRUE(plugin.library->libPath.endsWith("/dde-grand-search-daemon/libdde-grand-search-daemon.so"));
}

TEST(DaemonPlugin, start_with_uninitialize)
{
    DaemonPlugin plugin;
    ASSERT_EQ(plugin.library, nullptr);
    EXPECT_FALSE(plugin.start());
}

TEST(DaemonPlugin, start_ok)
{
    DaemonPlugin plugin;
    stub_ext::StubExt stub;
    stub.set_lamda(&DaemonLibrary::load,[](){
        return true;
    });

    bool started = false;
    stub.set_lamda(&DaemonLibrary::start, [&started](){
        started = true;
        return 0;
    });

    bool unload = false;
    stub.set_lamda(&DaemonLibrary::unload,[&unload](){
        unload = true;
        return;
    });

    plugin.initialize();
    ASSERT_NE(plugin.library, nullptr);

    EXPECT_TRUE(plugin.start());
    EXPECT_TRUE(started);
    EXPECT_FALSE(unload);
    EXPECT_NE(plugin.library, nullptr);
}

TEST(DaemonPlugin, start_flase)
{
    DaemonPlugin plugin;
    stub_ext::StubExt stub;
    stub.set_lamda(&DaemonLibrary::load,[](){
        return true;
    });

    bool unload = false;
    stub.set_lamda(&DaemonLibrary::unload,[&unload](){
        unload = true;
        return;
    });

    bool started = false;
    stub.set_lamda(&DaemonLibrary::start, [&started](){
        started = true;
        return -1;
    });
    plugin.initialize();
    ASSERT_NE(plugin.library, nullptr);

    EXPECT_FALSE(plugin.start());
    EXPECT_TRUE(started);
    EXPECT_TRUE(unload);
    EXPECT_EQ(plugin.library, nullptr);
}

TEST(DaemonPlugin, stop)
{
    DaemonPlugin plugin;
    stub_ext::StubExt stub;
    stub.set_lamda(&DaemonLibrary::load,[](){
        return true;
    });

    stub.set_lamda(&DaemonLibrary::start, [](){
        return 0;
    });

    bool unload = false;
    stub.set_lamda(&DaemonLibrary::unload,[&unload](){
        unload = true;
        return;
    });

    bool stop = false;
    stub.set_lamda(&DaemonLibrary::stop, [&stop](){
        stop = true;
        return 0;
    });

    plugin.initialize();

    EXPECT_FALSE(unload);
    EXPECT_FALSE(stop);
    plugin.stop();

    EXPECT_TRUE(unload);
    EXPECT_TRUE(stop);
}
