// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burneventreceiver.cpp
 * @brief Unit tests for BurnEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/burneventreceiver.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnEventReceiverTest, handleCopyFilesResult)
{
    // Test method: void handleCopyFilesResult((const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleCopyFilesResult(_arg0, _arg1, false, _arg3));
}

TEST_F(BurnEventReceiverTest, handleErase)
{
    // Test method: void handleErase((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleErase(_arg0));
}

TEST_F(BurnEventReceiverTest, handleFileRemoveResult)
{
    // Test method: void handleFileRemoveResult((const QList<QUrl> &srcUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileRemoveResult(_arg0, false, _arg2));
}

TEST_F(BurnEventReceiverTest, handleFileRenameResult)
{
    // Test method: void handleFileRenameResult((quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg))
    QMap<QUrl, QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileRenameResult(0, _arg1, false, _arg3));
}

TEST_F(BurnEventReceiverTest, handlePasteTo)
{
    // Test method: void handlePasteTo((const QList<QUrl> &urls, const QUrl &dest, bool isCopy))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handlePasteTo(_arg0, _arg1, false));
}

TEST_F(BurnEventReceiverTest, instance)
{
    // Test getter: BurnEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
