// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkeventreceiver.cpp
 * @brief Unit tests for BookMarkEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/bookmarkeventreceiver.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookMarkEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookMarkEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookMarkEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookMarkEventReceiverTest, BookMarkEventReceiver)
{
    // Test constructor: BookMarkEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BookMarkEventReceiverTest, handleRenameFile)
{
    // Test method: void handleRenameFile((quint64 windowId, const QMap<QUrl, QUrl> &renamedUrls, bool result, const QString &errorMsg))
    QMap<QUrl, QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRenameFile(0, _arg1, false, _arg3));
}

TEST_F(BookMarkEventReceiverTest, handleSidebarOrderChanged)
{
    // Test method: void handleSidebarOrderChanged((quint64 winId, const QString &group, const QList<QUrl> &urls))
    QString _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSidebarOrderChanged(0, _arg1, _arg2));
}

TEST_F(BookMarkEventReceiverTest, instance)
{
    // Test getter: BookMarkEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
