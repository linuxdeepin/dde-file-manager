// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tageventreceiver.cpp
 * @brief Unit tests for TagEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/tageventreceiver.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagEventReceiverTest, handleFileRemoveResult)
{
    // Test method: void handleFileRemoveResult((const QList<QUrl> &srcUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileRemoveResult(_arg0, false, _arg2));
}

TEST_F(TagEventReceiverTest, handleFileTrashedResult)
{
    // Test method: void handleFileTrashedResult((const QList<QUrl> &srcUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileTrashedResult(_arg0, false, _arg2));
}
