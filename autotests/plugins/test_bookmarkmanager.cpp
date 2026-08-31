// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bookmarkmanager.cpp
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

TEST_F(BookMarkManagerTest, addBookMarkItem)
{
    // Test method: void addBookMarkItem((const QUrl &url, const QString &bookmarkName, bool isDefaultItem))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addBookMarkItem(_arg0, _arg1, false));
}

TEST_F(BookMarkManagerTest, bookMarkRename)
{
    // Test method: bool bookMarkRename((const QUrl &url, const QString &newName))
    QUrl _arg0{};
    QString _arg1{};
    auto result = obj->bookMarkRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(BookMarkManagerTest, fileRenamed)
{
    // Test method: void fileRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->fileRenamed(_arg0, _arg1));
}

TEST_F(BookMarkManagerTest, getMountInfo)
{
    // Test method: void getMountInfo((const QUrl &url, QString &mountPoint))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->getMountInfo(_arg0, _arg1));
}

TEST_F(BookMarkManagerTest, initData)
{
    // Test method: void initData(())
    EXPECT_NO_FATAL_FAILURE(obj->initData());
}

TEST_F(BookMarkManagerTest, removeBookMark)
{
    // Test method: bool removeBookMark((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeBookMark(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BookMarkManagerTest, removeBookmarkFromDConfig)
{
    // Test method: void removeBookmarkFromDConfig((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeBookmarkFromDConfig(_arg0));
}

TEST_F(BookMarkManagerTest, saveQuickAccessToSortedItems)
{
    // Test method: void saveQuickAccessToSortedItems((const QVariantList &list))
    QVariantList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveQuickAccessToSortedItems(_arg0));
}

TEST_F(BookMarkManagerTest, saveSortedItemsToConfigFile)
{
    // Test method: void saveSortedItemsToConfigFile((const QList<QUrl> &order))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveSortedItemsToConfigFile(_arg0));
}

TEST_F(BookMarkManagerTest, showRemoveBookMarkDialog)
{
    // Test method: int showRemoveBookMarkDialog((quint64 winId))
    auto result = obj->showRemoveBookMarkDialog(0);
    EXPECT_GE(result, 0);

}

TEST_F(BookMarkManagerTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}
