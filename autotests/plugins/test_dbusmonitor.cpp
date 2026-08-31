// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbusmonitor.cpp
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

TEST_F(DBusMonitorTest, Enable)
{
    // Test getter: QDBusPendingReply<> Enable()
    auto result = obj->Enable();
    EXPECT_NO_FATAL_FAILURE({ obj->Enable(); });

}

TEST_F(DBusMonitorTest, connected)
{
    // Test bool getter: connected()
    bool result = obj->connected();
    EXPECT_FALSE(result);

}

TEST_F(DBusMonitorTest, height)
{
    // Test getter: quint16 height()
    auto result = obj->height();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusMonitorTest, rect)
{
    // Test getter: QRect rect()
    auto result = obj->rect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(DBusMonitorTest, staticInterfaceName)
{
    // Test getter: char staticInterfaceName()
    auto result = obj->staticInterfaceName();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusMonitorTest, staticServiceName)
{
    // Test getter: char staticServiceName()
    auto result = obj->staticServiceName();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusMonitorTest, width)
{
    // Test getter: quint16 width()
    auto result = obj->width();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusMonitorTest, x)
{
    // Test getter: qint16 x()
    auto result = obj->x();
    EXPECT_EQ(result, 0);

}

TEST_F(DBusMonitorTest, y)
{
    // Test getter: qint16 y()
    auto result = obj->y();
    EXPECT_EQ(result, 0);

}
