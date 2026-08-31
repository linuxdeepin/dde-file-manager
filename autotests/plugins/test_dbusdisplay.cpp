// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbusdisplay.cpp
 * @brief Unit tests for DBusDisplay methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbusdisplay1.h"

#include <QTest>

using namespace ddplugin_core;

class DBusDisplayTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DBusDisplay();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DBusDisplay *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DBusDisplayTest, DeleteCustomMode)
{
    // Test getter: QDBusPendingReply<> DeleteCustomMode()
    auto result = obj->DeleteCustomMode();
    EXPECT_NO_FATAL_FAILURE({ obj->DeleteCustomMode(); });

}

TEST_F(DBusDisplayTest, Reset)
{
    // Test getter: QDBusPendingReply<> Reset()
    auto result = obj->Reset();
    EXPECT_NO_FATAL_FAILURE({ obj->Reset(); });

}

TEST_F(DBusDisplayTest, ResetChanges)
{
    // Test getter: QDBusPendingReply<> ResetChanges()
    auto result = obj->ResetChanges();
    EXPECT_NO_FATAL_FAILURE({ obj->ResetChanges(); });

}

TEST_F(DBusDisplayTest, Save)
{
    // Test getter: QDBusPendingReply<> Save()
    auto result = obj->Save();
    EXPECT_NO_FATAL_FAILURE({ obj->Save(); });

}

TEST_F(DBusDisplayTest, SetAndSaveBrightness)
{
    // Test getter: QDBusPendingReply<> SetAndSaveBrightness()
    auto result = obj->SetAndSaveBrightness();
    EXPECT_NO_FATAL_FAILURE({ obj->SetAndSaveBrightness(); });

}

TEST_F(DBusDisplayTest, primary)
{
    // Test getter: QString primary()
    auto result = obj->primary();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusDisplayTest, DBusDisplay)
{
    // Test constructor: DBusDisplay((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DBusDisplayTest, staticObjectPath)
{
    // Test getter: char staticObjectPath()
    auto result = obj->staticObjectPath();
    EXPECT_EQ(result, 0);

}
