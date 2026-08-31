// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsereventreceiver_1.cpp
 * @brief Unit tests for SmbBrowserEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/smbbrowsereventreceiver.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowserEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowserEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserEventReceiverTest, SmbBrowserEventReceiver)
{
    // Test constructor: SmbBrowserEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SmbBrowserEventReceiverTest, cancelMoveToTrash)
{
    // Test method: bool cancelMoveToTrash((quint64, const QList<QUrl> &, const QUrl &rootUrl))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->cancelMoveToTrash(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventReceiverTest, detailViewIcon)
{
    // Test method: bool detailViewIcon((const QUrl &url, QString *iconName))
    QUrl _arg0{};
    auto result = obj->detailViewIcon(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventReceiverTest, hookAllowRepeatUrl)
{
    // Test method: bool hookAllowRepeatUrl((const QUrl &cur, const QUrl &pre))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->hookAllowRepeatUrl(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventReceiverTest, hookSetTabName)
{
    // Test method: bool hookSetTabName((const QUrl &url, QString *tabName))
    QUrl _arg0{};
    auto result = obj->hookSetTabName(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventReceiverTest, hookTitleBarAddrHandle)
{
    // Test method: bool hookTitleBarAddrHandle((QUrl *url))
    auto result = obj->hookTitleBarAddrHandle(nullptr);
    EXPECT_FALSE(result);

}
