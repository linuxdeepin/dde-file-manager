// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbusmonitor_1.cpp
 * @brief Unit tests for DBusMonitor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbusmonitor.h"

#include <QTest>

using namespace ddplugin_core;

class DBusMonitorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DBusMonitor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DBusMonitor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DBusMonitorTest, DBusMonitor)
{
    // Test constructor: DBusMonitor((const QString &objectPath,QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DBusMonitorTest, DBusMonitor_DBusMoni)
{
    // Test constructor: DBusMonitor((const QString &objectPath,QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DBusMonitorTest, SetMode)
{
    // Test getter: QDBusPendingReply<> SetMode()
    auto result = obj->SetMode();
    EXPECT_NO_FATAL_FAILURE({ obj->SetMode(); });

}

TEST_F(DBusMonitorTest, SetModeBySize)
{
    // Test getter: QDBusPendingReply<> SetModeBySize()
    auto result = obj->SetModeBySize();
    EXPECT_NO_FATAL_FAILURE({ obj->SetModeBySize(); });

}

TEST_F(DBusMonitorTest, SetPosition)
{
    // Test getter: QDBusPendingReply<> SetPosition()
    auto result = obj->SetPosition();
    EXPECT_NO_FATAL_FAILURE({ obj->SetPosition(); });

}

TEST_F(DBusMonitorTest, SetReflect)
{
    // Test getter: QDBusPendingReply<> SetReflect()
    auto result = obj->SetReflect();
    EXPECT_NO_FATAL_FAILURE({ obj->SetReflect(); });

}

TEST_F(DBusMonitorTest, SetRefreshRate)
{
    // Test getter: QDBusPendingReply<> SetRefreshRate()
    auto result = obj->SetRefreshRate();
    EXPECT_NO_FATAL_FAILURE({ obj->SetRefreshRate(); });

}

TEST_F(DBusMonitorTest, SetRotation)
{
    // Test getter: QDBusPendingReply<> SetRotation()
    auto result = obj->SetRotation();
    EXPECT_NO_FATAL_FAILURE({ obj->SetRotation(); });

}

TEST_F(DBusMonitorTest, __propertyChanged__)
{
    // Test method: void __propertyChanged__(())
    EXPECT_NO_FATAL_FAILURE(obj->__propertyChanged__());
}

TEST_F(DBusMonitorTest, __propertyChanged_____proper)
{
    // Test method: void __propertyChanged__(())
    EXPECT_NO_FATAL_FAILURE(obj->__propertyChanged__());
}

TEST_F(DBusMonitorTest, enabled)
{
    // Test bool getter: enabled()
    bool result = obj->enabled();
    EXPECT_FALSE(result);

}

TEST_F(DBusMonitorTest, enabled_enabled)
{
    // Test bool getter: enabled()
    bool result = obj->enabled();
    EXPECT_FALSE(result);

}

TEST_F(DBusMonitorTest, iD)
{
    // Test getter: uint iD()
    auto result = obj->iD();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusMonitorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusMonitorTest, name_name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusMonitorTest, DBusMonitor_Destructor)
{
    // Test method:  ~DBusMonitor(())
    EXPECT_NO_FATAL_FAILURE({ DBusMonitor *tmp = new DBusMonitor(); delete tmp; });
}

TEST_F(DBusMonitorTest, DBusMonitor_Destructor_xDBusMon)
{
    // Test method:  ~DBusMonitor(())
    EXPECT_NO_FATAL_FAILURE({ DBusMonitor *tmp = new DBusMonitor(); delete tmp; });
}
