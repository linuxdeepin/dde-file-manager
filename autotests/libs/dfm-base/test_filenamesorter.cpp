// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filenamesorter.cpp
 * @brief FileNameSorter 单元测试 —— 文件名排序工具 + 线程安全。
 *
 * 覆盖点：
 *   - sortKey 非空 / 确定性
 *   - sort 升序/降序、数值排序、空列表与单元素 no-op
 *   - sortUrls 按 fileName 排序
 *   - compare 升序/降序
 *   - sortByKey 模板排序
 *   - 与 CollationStrategyProvider 联动：排序结果稳定一致
 *   - 【线程安全】多线程并发 sort 不崩溃、结果正确
 *   - 【线程安全】多线程并发 sortKey + compare 不崩溃、结果正确
 */

#include <gtest/gtest.h>

#include <dfm-base/utils/filenamesorter.h>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

using namespace dfmbase;

class FileNameSorterTest : public testing::Test
{
protected:
    /// 期望数值排序的基准列表
    QStringList numericNames { "file10", "file2", "file1", "file20", "file3" };
    QStringList numericExpected { "file1", "file2", "file3", "file10", "file20" };
};

// sortKey 非空
TEST_F(FileNameSorterTest, SortKeyNonEmpty)
{
    EXPECT_FALSE(FileNameSorter::sortKey("test.txt").isEmpty());
    EXPECT_FALSE(FileNameSorter::sortKey("文件").isEmpty());
}

// sortKey 确定性
TEST_F(FileNameSorterTest, SortKeyDeterministic)
{
    EXPECT_EQ(FileNameSorter::sortKey("abc"), FileNameSorter::sortKey("abc"));
    EXPECT_EQ(FileNameSorter::sortKey("文件1"), FileNameSorter::sortKey("文件1"));
}

// sort 升序
TEST_F(FileNameSorterTest, SortAscending)
{
    QStringList names = { "banana", "apple", "cherry", "date" };
    FileNameSorter::sort(names, Qt::AscendingOrder);
    EXPECT_EQ(names, QStringList({ "apple", "banana", "cherry", "date" }));
}

// sort 降序
TEST_F(FileNameSorterTest, SortDescending)
{
    QStringList names = { "banana", "apple", "cherry", "date" };
    FileNameSorter::sort(names, Qt::DescendingOrder);
    EXPECT_EQ(names, QStringList({ "date", "cherry", "banana", "apple" }));
}

// sort 数值排序（file2 < file10）
TEST_F(FileNameSorterTest, SortNumericOrdering)
{
    QStringList names = numericNames;
    FileNameSorter::sort(names);
    EXPECT_EQ(names, numericExpected);
}

// sort 默认升序
TEST_F(FileNameSorterTest, SortDefaultIsAscending)
{
    QStringList names = { "c", "a", "b" };
    FileNameSorter::sort(names);   // 默认 Qt::AscendingOrder
    EXPECT_EQ(names, QStringList({ "a", "b", "c" }));
}

// sort 空列表与单元素 no-op
TEST_F(FileNameSorterTest, SortEmptyAndSingleNoOp)
{
    QStringList empty;
    FileNameSorter::sort(empty);
    EXPECT_TRUE(empty.isEmpty());

    QStringList single = { "only.txt" };
    FileNameSorter::sort(single);
    EXPECT_EQ(single, QStringList({ "only.txt" }));
}

// sort 稳定性：相同排序键的元素保持相对顺序（std::stable_sort）
TEST_F(FileNameSorterTest, SortIsStable)
{
    // 两个 "same" 项应保持输入相对顺序（stable_sort 保证）
    QStringList names = { "same", "aaa", "same", "bbb" };
    FileNameSorter::sort(names);
    EXPECT_EQ(names, QStringList({ "aaa", "bbb", "same", "same" }));
}

// compare 升序/降序
TEST_F(FileNameSorterTest, CompareAscendingAndDescending)
{
    EXPECT_TRUE(FileNameSorter::compare("apple", "banana"));   // apple < banana
    EXPECT_FALSE(FileNameSorter::compare("banana", "apple"));   // banana > apple
    // 降序：banana 应排在 apple 前
    EXPECT_TRUE(FileNameSorter::compare("banana", "apple", Qt::DescendingOrder));
    EXPECT_FALSE(FileNameSorter::compare("apple", "banana", Qt::DescendingOrder));
}

// sortUrls 按 fileName 排序
TEST_F(FileNameSorterTest, SortUrlsByFileName)
{
    QList<QUrl> urls = {
        QUrl::fromLocalFile("/path/banana.txt"),
        QUrl::fromLocalFile("/path/apple.txt"),
        QUrl::fromLocalFile("/path/cherry.txt"),
    };
    FileNameSorter::sortUrls(urls);
    ASSERT_EQ(urls.size(), 3);
    EXPECT_EQ(urls[0].fileName(), "apple.txt");
    EXPECT_EQ(urls[1].fileName(), "banana.txt");
    EXPECT_EQ(urls[2].fileName(), "cherry.txt");
}

// sortUrls 降序
TEST_F(FileNameSorterTest, SortUrlsDescending)
{
    QList<QUrl> urls = {
        QUrl::fromLocalFile("/path/banana"),
        QUrl::fromLocalFile("/path/apple"),
        QUrl::fromLocalFile("/path/cherry"),
    };
    FileNameSorter::sortUrls(urls, Qt::DescendingOrder);
    ASSERT_EQ(urls.size(), 3);
    EXPECT_EQ(urls[0].fileName(), "cherry");
    EXPECT_EQ(urls[2].fileName(), "apple");
}

// sortUrls 空与单元素 no-op
TEST_F(FileNameSorterTest, SortUrlsEmptyAndSingleNoOp)
{
    QList<QUrl> empty;
    FileNameSorter::sortUrls(empty);
    EXPECT_TRUE(empty.isEmpty());

    QList<QUrl> single = { QUrl::fromLocalFile("/x/only") };
    FileNameSorter::sortUrls(single);
    ASSERT_EQ(single.size(), 1);
    EXPECT_EQ(single[0].fileName(), "only");
}

// sortByKey 模板排序
TEST_F(FileNameSorterTest, SortByKeyTemplate)
{
    QVector<QPair<QString, QByteArray>> items = {
        { "banana", FileNameSorter::sortKey("banana") },
        { "apple", FileNameSorter::sortKey("apple") },
        { "cherry", FileNameSorter::sortKey("cherry") },
    };
    FileNameSorter::sortByKey(items,
                              [](const auto &item) { return item.second; });
    ASSERT_EQ(items.size(), 3);
    EXPECT_EQ(items[0].first, "apple");
    EXPECT_EQ(items[1].first, "banana");
    EXPECT_EQ(items[2].first, "cherry");
}

// sortByKey 模板降序
TEST_F(FileNameSorterTest, SortByKeyTemplateDescending)
{
    QVector<QPair<QString, QByteArray>> items = {
        { "banana", FileNameSorter::sortKey("banana") },
        { "apple", FileNameSorter::sortKey("apple") },
        { "cherry", FileNameSorter::sortKey("cherry") },
    };
    FileNameSorter::sortByKey(items,
                              [](const auto &item) { return item.second; },
                              Qt::DescendingOrder);
    ASSERT_EQ(items.size(), 3);
    EXPECT_EQ(items[0].first, "cherry");
    EXPECT_EQ(items[2].first, "apple");
}

// ───────────────────────── 线程安全测试 ─────────────────────────

// 【线程安全】多线程并发 sort 不崩溃、结果正确
TEST_F(FileNameSorterTest, ConcurrentSortIsSafeAndCorrect)
{
    const int numThreads = 8;
    const int iterations = 100;
    std::atomic<int> failures { 0 };
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations; ++i) {
                QStringList names = numericNames;   // 每线程独立副本
                FileNameSorter::sort(names);
                if (names != numericExpected)
                    ++failures;
            }
        });
    }
    for (auto &th : threads)
        th.join();

    EXPECT_EQ(failures.load(), 0) << "并发排序结果应全部正确";
}

// 【线程安全】多线程并发 sortKey + compare 不崩溃、结果正确
TEST_F(FileNameSorterTest, ConcurrentSortKeyAndCompareSafe)
{
    const int numThreads = 8;
    std::atomic<int> failures { 0 };
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 200; ++i) {
                QByteArray k = FileNameSorter::sortKey("test");
                if (k.isEmpty()) {
                    ++failures;
                    break;
                }
                // a < b 升序应返回 true
                if (!FileNameSorter::compare("a", "b")) {
                    ++failures;
                }
                // 数值序：file2 < file10
                if (!FileNameSorter::compare("file2", "file10")) {
                    ++failures;
                }
            }
        });
    }
    for (auto &th : threads)
        th.join();

    EXPECT_EQ(failures.load(), 0);
}

// 【线程安全】并发 sortUrls 不崩溃、结果正确
TEST_F(FileNameSorterTest, ConcurrentSortUrlsSafe)
{
    const int numThreads = 6;
    std::atomic<int> failures { 0 };
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 50; ++i) {
                QList<QUrl> urls = {
                    QUrl::fromLocalFile("/d/file10"),
                    QUrl::fromLocalFile("/d/file2"),
                    QUrl::fromLocalFile("/d/file1"),
                };
                FileNameSorter::sortUrls(urls);
                if (urls[0].fileName() != "file1" || urls[1].fileName() != "file2"
                    || urls[2].fileName() != "file10")
                    ++failures;
            }
        });
    }
    for (auto &th : threads)
        th.join();

    EXPECT_EQ(failures.load(), 0);
}

// 【线程安全】并发 sort + sortKey 混合调用不崩溃
TEST_F(FileNameSorterTest, ConcurrentMixedOperationsSafe)
{
    const int numThreads = 8;
    std::atomic<int> failures { 0 };
    std::atomic<int> crashes { 0 };
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([t, &failures, &crashes]() {
            try {
                for (int i = 0; i < 100; ++i) {
                    if (t % 2 == 0) {
                        QStringList names = { "c", "a", "b", "a" };
                        FileNameSorter::sort(names);
                        if (names != QStringList({ "a", "a", "b", "c" }))
                            ++failures;
                    } else {
                        if (FileNameSorter::sortKey("x").isEmpty())
                            ++failures;
                        if (!FileNameSorter::compare("a", "b"))
                            ++failures;
                    }
                }
            } catch (...) {
                ++crashes;
            }
        });
    }
    for (auto &th : threads)
        th.join();

    EXPECT_EQ(crashes.load(), 0);
    EXPECT_EQ(failures.load(), 0);
}
