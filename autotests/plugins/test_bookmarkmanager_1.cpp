// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkmanager_1.cpp
 * @brief Unit tests for BookMarkManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "controller/bookmarkmanager.h"

#include <QTest>

using namespace dfmplugin_bookmark;

class BookMarkManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BookMarkManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BookMarkManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BookMarkManagerTest, BookMarkManager)
{
    // Test constructor: BookMarkManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BookMarkManagerTest, addBookmarkToDConfig)
{
    // Test method: void addBookmarkToDConfig((const QVariantMap &data))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addBookmarkToDConfig(_arg0));
}

TEST_F(BookMarkManagerTest, addQuickAccessItemsFromConfig)
{
    // Test method: void addQuickAccessItemsFromConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->addQuickAccessItemsFromConfig());
}

TEST_F(BookMarkManagerTest, getBookMarkDataMap)
{
    // Test getter: QMap<QUrl, BookmarkData> getBookMarkDataMap()
    auto result = obj->getBookMarkDataMap();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BookMarkManagerTest, isItemDuplicated)
{
    // Test method: bool isItemDuplicated((const BookmarkData &data))
    BookmarkData _arg0{};
    auto result = obj->isItemDuplicated(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BookMarkManagerTest, renameBookmarkToDConfig)
{
    // Test method: void renameBookmarkToDConfig((const QString &oldName, const QString &newName))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->renameBookmarkToDConfig(_arg0, _arg1));
}

TEST_F(BookMarkManagerTest, updateBookmarkUrlToDconfig)
{
    // Test method: void updateBookmarkUrlToDconfig((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateBookmarkUrlToDconfig(_arg0, _arg1));
}
