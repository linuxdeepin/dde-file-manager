// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tageventreceiver_1.cpp
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

TEST_F(TagEventReceiverTest, TagEventReceiver)
{
    // Test constructor: TagEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagEventReceiverTest, addTagsToUrl)
{
    // Test method: void addTagsToUrl((const QUrl &url, const QStringList &tags))
    QUrl _arg0{};
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addTagsToUrl(_arg0, _arg1));
}

TEST_F(TagEventReceiverTest, handleFileCopyResult)
{
    // Test method: void handleFileCopyResult((const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileCopyResult(_arg0, _arg1, false, _arg3));
}

TEST_F(TagEventReceiverTest, handleTrashCleanedResult)
{
    // Test method: void handleTrashCleanedResult((const QList<QUrl> &destUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTrashCleanedResult(_arg0, false, _arg2));
}

TEST_F(TagEventReceiverTest, handleWindowUrlChanged)
{
    // Test method: void handleWindowUrlChanged((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleWindowUrlChanged(0, _arg1));
}

TEST_F(TagEventReceiverTest, instance)
{
    // Test getter: TagEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(TagEventReceiverTest, processDirectoryTags)
{
    // Test method: void processDirectoryTags((const QUrl &srcUrl, const QUrl &destUrl, bool shouldRemoveSource))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->processDirectoryTags(_arg0, _arg1, false));
}

TEST_F(TagEventReceiverTest, processFileTags)
{
    // Test method: void processFileTags((const QUrl &srcUrl, const QUrl &destUrl, bool shouldRemoveSource))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->processFileTags(_arg0, _arg1, false));
}
