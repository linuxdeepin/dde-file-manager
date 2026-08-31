// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filenamesorter.cpp
 * @brief Unit tests for FileNameSorter (filenamesorter.cpp)
 */

#include <gtest/gtest.h>
#include <QCollator>
#include <QStringList>
#include <QUrl>

#include <dfm-base/utils/filenamesorter.h>

using namespace dfmbase;

TEST(FileNameSorterTest, CollatorReturnsSameInstance)
{
    QCollator &c1 = FileNameSorter::collator();
    QCollator &c2 = FileNameSorter::collator();
    EXPECT_EQ(&c1, &c2);
}

TEST(FileNameSorterTest, CollatorHasNumericMode)
{
    QCollator &c = FileNameSorter::collator();
    EXPECT_TRUE(c.numericMode());
}

TEST(FileNameSorterTest, CollatorIsCaseSensitive)
{
    QCollator &c = FileNameSorter::collator();
    EXPECT_EQ(c.caseSensitivity(), Qt::CaseSensitive);
}

TEST(FileNameSorterTest, SortKeyProducesComparableKey)
{
    QCollatorSortKey k1 = FileNameSorter::sortKey("file10");
    QCollatorSortKey k2 = FileNameSorter::sortKey("file2");
    EXPECT_TRUE(k2 < k1);
}

TEST(FileNameSorterTest, SortAscendingNaturalOrder)
{
    QStringList names { "file10", "file2", "file1", "file20" };
    FileNameSorter::sort(names, Qt::AscendingOrder);
    EXPECT_EQ(names, QStringList({ "file1", "file2", "file10", "file20" }));
}

TEST(FileNameSorterTest, SortDescendingNaturalOrder)
{
    QStringList names { "file10", "file2", "file1", "file20" };
    FileNameSorter::sort(names, Qt::DescendingOrder);
    EXPECT_EQ(names, QStringList({ "file20", "file10", "file2", "file1" }));
}

TEST(FileNameSorterTest, SortSingleElementNoOp)
{
    QStringList names { "only" };
    FileNameSorter::sort(names, Qt::AscendingOrder);
    EXPECT_EQ(names, QStringList({ "only" }));
}

TEST(FileNameSorterTest, SortEmptyListNoOp)
{
    QStringList names;
    FileNameSorter::sort(names, Qt::AscendingOrder);
    EXPECT_TRUE(names.isEmpty());
}

TEST(FileNameSorterTest, SortUrlsAscending)
{
    QList<QUrl> urls {
        QUrl("file:///dir/file10.txt"),
        QUrl("file:///dir/file2.txt"),
        QUrl("file:///dir/file1.txt"),
    };
    FileNameSorter::sortUrls(urls, Qt::AscendingOrder);
    EXPECT_EQ(urls[0].fileName(), QString("file1.txt"));
    EXPECT_EQ(urls[1].fileName(), QString("file2.txt"));
    EXPECT_EQ(urls[2].fileName(), QString("file10.txt"));
}

TEST(FileNameSorterTest, SortUrlsDescending)
{
    QList<QUrl> urls {
        QUrl("file:///dir/file2.txt"),
        QUrl("file:///dir/file10.txt"),
        QUrl("file:///dir/file1.txt"),
    };
    FileNameSorter::sortUrls(urls, Qt::DescendingOrder);
    EXPECT_EQ(urls[0].fileName(), QString("file10.txt"));
}

TEST(FileNameSorterTest, CompareAscending)
{
    EXPECT_TRUE(FileNameSorter::compare("file2", "file10", Qt::AscendingOrder));
    EXPECT_FALSE(FileNameSorter::compare("file10", "file2", Qt::AscendingOrder));
}

TEST(FileNameSorterTest, CompareDescending)
{
    EXPECT_TRUE(FileNameSorter::compare("file10", "file2", Qt::DescendingOrder));
    EXPECT_FALSE(FileNameSorter::compare("file2", "file10", Qt::DescendingOrder));
}

TEST(FileNameSorterTest, SortByKeyAscending)
{
    using Item = QPair<QString, QCollatorSortKey>;
    QVector<Item> items {
        { "b", FileNameSorter::sortKey("b") },
        { "a", FileNameSorter::sortKey("a") },
        { "c", FileNameSorter::sortKey("c") },
    };
    FileNameSorter::sortByKey(items, [](const Item &it) { return it.second; }, Qt::AscendingOrder);
    EXPECT_EQ(items[0].first, QString("a"));
    EXPECT_EQ(items[1].first, QString("b"));
    EXPECT_EQ(items[2].first, QString("c"));
}

TEST(FileNameSorterTest, SortByKeyDescending)
{
    using Item = QPair<QString, QCollatorSortKey>;
    QVector<Item> items {
        { "b", FileNameSorter::sortKey("b") },
        { "a", FileNameSorter::sortKey("a") },
        { "c", FileNameSorter::sortKey("c") },
    };
    FileNameSorter::sortByKey(items, [](const Item &it) { return it.second; }, Qt::DescendingOrder);
    EXPECT_EQ(items[0].first, QString("c"));
    EXPECT_EQ(items[1].first, QString("b"));
    EXPECT_EQ(items[2].first, QString("a"));
}


TEST(FileNameSorterTest, compare)
{
    // compare
    SUCCEED();
}

TEST(FileNameSorterTest, sort)
{
    // sort
    SUCCEED();
}

TEST(FileNameSorterTest, sortKey)
{
    // sortKey
    SUCCEED();
}

TEST(FileNameSorterTest, sortUrls)
{
    // sortUrls
    SUCCEED();
}
