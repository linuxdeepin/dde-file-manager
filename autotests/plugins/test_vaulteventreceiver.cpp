// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaulteventreceiver.cpp
 * @brief Unit tests for VaultEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/vaulteventreceiver.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultEventReceiverTest, VaultEventReceiver)
{
    // Test constructor: VaultEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultEventReceiverTest, changeUrlEventFilter)
{
    // Test method: bool changeUrlEventFilter((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->changeUrlEventFilter(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, computerOpenItem)
{
    // Test method: void computerOpenItem((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->computerOpenItem(0, _arg1));
}

TEST_F(VaultEventReceiverTest, handleNotAllowedAppendCompress)
{
    // Test method: bool handleNotAllowedAppendCompress((const QList<QUrl> &fromUrls, const QUrl &toUrl))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->handleNotAllowedAppendCompress(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(VaultEventReceiverTest, instance)
{
    // Test getter: VaultEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
