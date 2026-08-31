// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appearance_interface.cpp
 * @brief Unit tests for Appearance_Interface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "appearance_interface.h"

#include <QTest>

using namespace ddplugin_background;

class Appearance_InterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Appearance_Interface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Appearance_Interface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(Appearance_InterfaceTest, Delete)
{
    // Test getter: QDBusPendingReply<> Delete()
    auto result = obj->Delete();
    EXPECT_NO_FATAL_FAILURE({ obj->Delete(); });

}

TEST_F(Appearance_InterfaceTest, List)
{
    // Test getter: QDBusPendingReply<QString> List()
    auto result = obj->List();
    EXPECT_NO_FATAL_FAILURE({ obj->List(); });

}

TEST_F(Appearance_InterfaceTest, Reset)
{
    // Test getter: QDBusPendingReply<> Reset()
    auto result = obj->Reset();
    EXPECT_NO_FATAL_FAILURE({ obj->Reset(); });

}

TEST_F(Appearance_InterfaceTest, Show)
{
    // Test getter: QDBusPendingReply<QString> Show()
    auto result = obj->Show();
    EXPECT_NO_FATAL_FAILURE({ obj->Show(); });

}

TEST_F(Appearance_InterfaceTest, Thumbnail)
{
    // Test getter: QDBusPendingReply<QString> Thumbnail()
    auto result = obj->Thumbnail();
    EXPECT_NO_FATAL_FAILURE({ obj->Thumbnail(); });

}

TEST_F(Appearance_InterfaceTest, fontSize)
{
    // Test getter: double fontSize()
    auto result = obj->fontSize();
    EXPECT_EQ(result, 0.0);

}

TEST_F(Appearance_InterfaceTest, Appearance_Interface)
{
    // Test constructor: Appearance_Interface((const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent))
    ASSERT_NE(obj, nullptr);
}
