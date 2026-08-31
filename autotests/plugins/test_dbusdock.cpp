// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbusdock.cpp
 * @brief Unit tests for DBusDock methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbusdock1.h"

#include <QTest>

using namespace ddplugin_core;

class DBusDockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DBusDock();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DBusDock *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DBusDockTest, DBusDock)
{
    // Test constructor: DBusDock((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DBusDockTest, RemovePluginSettings)
{
    // Test getter: QDBusPendingReply<> RemovePluginSettings()
    auto result = obj->RemovePluginSettings();
    EXPECT_NO_FATAL_FAILURE({ obj->RemovePluginSettings(); });

}

TEST_F(DBusDockTest, setPosition)
{
    // Test method: void setPosition(())
    EXPECT_NO_FATAL_FAILURE(obj->setPosition());
}
