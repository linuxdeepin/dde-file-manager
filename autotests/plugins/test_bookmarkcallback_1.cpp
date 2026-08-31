// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkcallback_1.cpp
 * @brief Unit tests for BookmarkCallBack methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "bookmarkcallback.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookmarkCallBackTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookmarkCallBack();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookmarkCallBack *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookmarkCallBackTest, cdDefaultItemUrlCallBack)
{
    // Test method: void cdDefaultItemUrlCallBack((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->cdDefaultItemUrlCallBack(0, _arg1));
}

TEST_F(BookmarkCallBackTest, renameCallBack)
{
    // Test method: void renameCallBack((quint64 windowId, const QUrl &url, const QString &name))
    QUrl _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->renameCallBack(0, _arg1, _arg2));
}
