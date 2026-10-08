// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filenamesorter.cpp
 * @brief Unit tests for FileNameSorter (utils/filenamesorter.cpp).
 *
 * FileNameSorter is the surviving successor of the SortUtils module
 * (introduced by the PMS 322155 fix and later refactored away in
 * 1944582e5); the numeric-aware custom sorting contract is exercised here.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QStringList>
#include <QList>

#include <dfm-base/utils/filenamesorter.h>

using namespace dfmbase;

// PMS:322155 自定义排序数字感知：文件名内嵌数字按数值排序（b2 在 b10 前），
// 而非字典序（b10 在 b2 前）。该契约源自原 SortUtils::isNumOrChar 修复。
TEST(FileNameSorterTest, BUG322155_NumericAwareAscendingOrder)
{
    QStringList names { "b10.txt", "b2.txt", "b1.txt" };
    FileNameSorter::sort(names);
    EXPECT_EQ(names, (QStringList { "b1.txt", "b2.txt", "b10.txt" }));
}

// PMS:322155 URL 列表批量排序：按文件名数字感知排序且保持升序契约
TEST(FileNameSorterTest, BUG322155_SortUrlsNumericAware)
{
    QList<QUrl> urls { QUrl("file:///b10.txt"), QUrl("file:///b2.txt"), QUrl("file:///b1.txt") };
    FileNameSorter::sortUrls(urls);
    const QList<QUrl> expected { QUrl("file:///b1.txt"),
                                 QUrl("file:///b2.txt"),
                                 QUrl("file:///b10.txt") };
    EXPECT_EQ(urls, expected);
}

// PMS:322155 compare 与降序排列：数字感知比较在两侧应保持一致，降序翻转结果
TEST(FileNameSorterTest, BUG322155_CompareAndDescendingOrder)
{
    EXPECT_TRUE(FileNameSorter::compare("a2.txt", "a10.txt"));
    EXPECT_FALSE(FileNameSorter::compare("a10.txt", "a2.txt"));

    QStringList names { "b1", "b10" };
    FileNameSorter::sort(names, Qt::DescendingOrder);
    EXPECT_EQ(names, (QStringList { "b10", "b1" }));
}
