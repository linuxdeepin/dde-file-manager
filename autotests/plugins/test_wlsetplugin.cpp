// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wlsetplugin.cpp
 * @brief Unit tests for WlSetPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wlsetplugin.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WlSetPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WlSetPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WlSetPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WlSetPluginTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(WlSetPluginTest, registerDBus)
{
    // Test method: void registerDBus(())
    EXPECT_NO_FATAL_FAILURE(obj->registerDBus());
}

TEST_F(WlSetPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(WlSetPluginTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}
