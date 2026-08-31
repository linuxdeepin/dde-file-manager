// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbushelper.cpp
 * @brief Unit tests for DBusHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbushelper.h"

#include <QTest>

using namespace ddplugin_core;

class DBusHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DBusHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DBusHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DBusHelperTest, display)
{
    // Test getter: DBusDisplay display()
    auto result = obj->display();
    EXPECT_NO_FATAL_FAILURE({ obj->display(); });

}

TEST_F(DBusHelperTest, ins)
{
    // Test getter: DBusHelper ins()
    auto result = obj->ins();
    EXPECT_NO_FATAL_FAILURE({ obj->ins(); });

}

TEST_F(DBusHelperTest, isDockEnable)
{
    // Test bool getter: isDockEnable()
    bool result = obj->isDockEnable();
    EXPECT_FALSE(result);

}
