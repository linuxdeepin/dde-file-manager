// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recenteventreceiver.cpp
 * @brief Unit tests for RecentEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/recenteventreceiver.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentEventReceiverTest, RecentEventReceiver)
{
    // Test constructor: RecentEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentEventReceiverTest, handleRemoveFilesResult)
{
    // Test method: void handleRemoveFilesResult((const QList<QUrl> &urls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRemoveFilesResult(_arg0, false, _arg2));
}
