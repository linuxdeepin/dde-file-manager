// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualopenwithplugin.cpp
 * @brief Unit tests for VirtualOpenWithPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/virtualopenwithplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualOpenWithPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualOpenWithPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualOpenWithPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualOpenWithPluginTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(VirtualOpenWithPluginTest, regViewToPropertyDialog)
{
    // Test method: void regViewToPropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->regViewToPropertyDialog());
}

TEST_F(VirtualOpenWithPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
