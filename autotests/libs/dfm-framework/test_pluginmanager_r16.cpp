// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSharedPointer>

#include <dfm-framework/lifecycle/pluginmanager.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>

using namespace dpf;

TEST(PluginManagerR16Test, CtorAndDtorSafe)
{
    PluginManager pm;
    SUCCEED();
}

TEST(PluginManagerR16Test, PluginIIDsEmptyByDefault)
{
    PluginManager pm;
    EXPECT_TRUE(pm.pluginIIDs().isEmpty());
}

TEST(PluginManagerR16Test, PluginPathsEmptyByDefault)
{
    PluginManager pm;
    EXPECT_TRUE(pm.pluginPaths().isEmpty());
}

TEST(PluginManagerR16Test, BlackListEmptyByDefault)
{
    PluginManager pm;
    EXPECT_TRUE(pm.blackList().isEmpty());
}

TEST(PluginManagerR16Test, ReadQueueEmptyByDefault)
{
    PluginManager pm;
    EXPECT_TRUE(pm.readQueue().isEmpty());
}

TEST(PluginManagerR16Test, LoadQueueEmptyByDefault)
{
    PluginManager pm;
    EXPECT_TRUE(pm.loadQueue().isEmpty());
}

TEST(PluginManagerR16Test, StopPluginsOnEmptyQueue)
{
    PluginManager pm;
    EXPECT_NO_FATAL_FAILURE({ pm.stopPlugins(); });
}

TEST(PluginManagerR16Test, ReadPluginsFalseWithNoIIDs)
{
    PluginManager pm;
    EXPECT_FALSE(pm.readPlugins());
}

TEST(PluginManagerR16Test, LoadPluginsTrueOnEmptyQueue)
{
    PluginManager pm;
    EXPECT_TRUE(pm.loadPlugins());
}

TEST(PluginManagerR16Test, D0DestructorPath)
{
    auto *ptr = new PluginManager();
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}
