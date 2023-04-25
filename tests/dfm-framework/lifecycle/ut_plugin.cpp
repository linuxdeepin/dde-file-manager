// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testplugin.h"

#include <gtest/gtest.h>

class UT_Plugin : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    TestPlugin plugin;
};

TEST_F(UT_Plugin, test_plugin_obj)
{
    EXPECT_FALSE(plugin.initialized);
    EXPECT_FALSE(plugin.stopped);
    plugin.initialize();
    EXPECT_TRUE(plugin.start());
    plugin.stop();
    EXPECT_TRUE(plugin.initialized);
    EXPECT_TRUE(plugin.stopped);
}
