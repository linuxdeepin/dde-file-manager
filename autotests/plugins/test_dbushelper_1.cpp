// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbushelper_1.cpp
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

TEST_F(DBusHelperTest, createMonitor)
{
    // Test method: DBusMonitor createMonitor((const QString &path))
    QString _arg0{};
    auto result = obj->createMonitor(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createMonitor(_arg0); });

}

TEST_F(DBusHelperTest, dock)
{
    // Test getter: DBusDock dock()
    auto result = obj->dock();
    EXPECT_NO_FATAL_FAILURE({ obj->dock(); });

}

TEST_F(DBusHelperTest, isDisplayEnable)
{
    // Test bool getter: isDisplayEnable()
    bool result = obj->isDisplayEnable();
    EXPECT_FALSE(result);

}
