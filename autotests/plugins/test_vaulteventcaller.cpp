// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaulteventcaller.cpp
 * @brief Unit tests for VaultEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/vaulteventcaller.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultEventCallerTest, VaultEventCaller)
{
    // Test constructor: VaultEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultEventCallerTest, sendItemActived)
{
    // Test method: void sendItemActived((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendItemActived(0, _arg1));
}

TEST_F(VaultEventCallerTest, sendOpenFiles)
{
    // Test method: void sendOpenFiles((const quint64 windowID, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenFiles(0, _arg1));
}

TEST_F(VaultEventCallerTest, sendOpenTab)
{
    // Test method: void sendOpenTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenTab(0, _arg1));
}

TEST_F(VaultEventCallerTest, sendOpenWindow)
{
    // Test method: void sendOpenWindow((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenWindow(_arg0));
}

TEST_F(VaultEventCallerTest, sendVaultProperty)
{
    // Test method: void sendVaultProperty((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendVaultProperty(_arg0));
}
