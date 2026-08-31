// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewsorter_1.cpp
 * @brief Unit tests for FileViewSorter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewsorter.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewSorterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewSorter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewSorter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewSorterTest, FileViewSorter)
{
    // Test constructor: FileViewSorter(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileViewSorterTest, encodeMimeTypeSortKey)
{
    // Test method: QString encodeMimeTypeSortKey((const QString &mimeType, const QString &fileName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->encodeMimeTypeSortKey(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewSorterTest, findInsertPosition)
{
    // Test method: int findInsertPosition((const QUrl &url, const QList<QUrl> &sortedList))
    QUrl _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->findInsertPosition(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewSorterTest, generateSortKeyString)
{
    // Test method: QString generateSortKeyString((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->generateSortKeyString(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewSorterTest, getFileDisplayName)
{
    // Test method: QString getFileDisplayName((const QUrl &url, const FileItemDataPointer &itemData))
    QUrl _arg0{};
    FileItemDataPointer _arg1{};
    auto result = obj->getFileDisplayName(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewSorterTest, getMimeTypeGroupRank)
{
    // Test method: int getMimeTypeGroupRank((const QString &mimeType))
    QString _arg0{};
    auto result = obj->getMimeTypeGroupRank(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewSorterTest, isDir)
{
    // Test method: bool isDir((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isDir(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewSorterTest, setContext)
{
    // Test setter: void setContext((const SortContext &context))
    SortContext _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setContext(_arg0));
}

TEST_F(FileViewSorterTest, FileViewSorter_Destructor)
{
    // Test method:  ~FileViewSorter(())
    EXPECT_NO_FATAL_FAILURE({ FileViewSorter *tmp = new FileViewSorter(); delete tmp; });
}
