// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbussessionmanager_1.cpp
 * @brief Unit tests for DBusSessionManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/dbussessionmanager.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class DBusSessionManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DBusSessionManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DBusSessionManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DBusSessionManagerTest, DBusSessionManager)
{
    // Test constructor: DBusSessionManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DBusSessionManagerTest, __propertyChanged__)
{
    // Test method: void __propertyChanged__(())
    EXPECT_NO_FATAL_FAILURE(obj->__propertyChanged__());
}

TEST_F(DBusSessionManagerTest, stage)
{
    // Test getter: int stage()
    auto result = obj->stage();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusSessionManagerTest, staticInterfaceName)
{
    // Test getter: char staticInterfaceName()
    auto result = obj->staticInterfaceName();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusSessionManagerTest, staticObjectPath)
{
    // Test getter: char staticObjectPath()
    auto result = obj->staticObjectPath();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusSessionManagerTest, staticServiceName)
{
    // Test getter: char staticServiceName()
    auto result = obj->staticServiceName();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusSessionManagerTest, DBusSessionManager_Destructor)
{
    // Test method:  ~DBusSessionManager(())
    EXPECT_NO_FATAL_FAILURE({ DBusSessionManager *tmp = new DBusSessionManager(); delete tmp; });
}
