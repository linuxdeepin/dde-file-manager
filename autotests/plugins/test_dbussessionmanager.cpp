// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbussessionmanager.cpp
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

TEST_F(DBusSessionManagerTest, currentUid)
{
    // Test getter: QString currentUid()
    auto result = obj->currentUid();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DBusSessionManagerTest, locked)
{
    // Test bool getter: locked()
    bool result = obj->locked();
    EXPECT_FALSE(result);

}
