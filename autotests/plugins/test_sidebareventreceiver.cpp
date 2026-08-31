// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebareventreceiver.cpp
 * @brief Unit tests for SideBarEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/sidebareventreceiver.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarEventReceiverTest, handleAliasSettingChanged)
{
    // Test method: void handleAliasSettingChanged((const QString &group, const QString &key, const QVariant &value))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleAliasSettingChanged(_arg0, _arg1, _arg2));
}

TEST_F(SideBarEventReceiverTest, handleItemAdd)
{
    // Test method: bool handleItemAdd((const QUrl &url, const QVariantMap &properties))
    QUrl _arg0{};
    QVariantMap _arg1{};
    auto result = obj->handleItemAdd(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SideBarEventReceiverTest, handleItemRemove)
{
    // Test method: bool handleItemRemove((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->handleItemRemove(_arg0);
    EXPECT_FALSE(result);

}
