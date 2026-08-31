// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowsereventreceiver.cpp
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

TEST_F(SmbBrowserEventReceiverTest, cancelDelete)
{
    // Test method: bool cancelDelete((quint64, const QList<QUrl> &urls, const QUrl &rootUrl))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->cancelDelete(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventReceiverTest, getOriginalUri)
{
    // Test method: bool getOriginalUri((const QUrl &in, QUrl *out))
    QUrl _arg0{};
    auto result = obj->getOriginalUri(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventReceiverTest, hookCopyFilePath)
{
    // Test method: bool hookCopyFilePath((quint64, const QList<QUrl> &urlList, const QUrl &rootUrl))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    auto result = obj->hookCopyFilePath(0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserEventReceiverTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}
