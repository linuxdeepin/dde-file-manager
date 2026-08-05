// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_highlightprovider.cpp
 * @brief Unit tests for HighlightProvider (highlightprovider.cpp)
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>

#include <dfm-base/utils/highlightprovider.h>

using namespace dfmbase;

TEST(HighlightProviderTest, InstanceNotNull)
{
    EXPECT_NE(HighlightProvider::instance(), nullptr);
}

TEST(HighlightProviderTest, SetAndGetPositioningMaxLength)
{
    auto *p = HighlightProvider::instance();
    p->setPositioningMaxLength(200);
    EXPECT_EQ(p->positioningMaxLength(), 200);
    p->setPositioningMaxLength(0);
    EXPECT_EQ(p->positioningMaxLength(), 0);
}

TEST(HighlightProviderTest, RequestHighlightWithoutCallbackDoesNotCrash)
{
    auto *p = HighlightProvider::instance();
    p->setFetchCallback(nullptr);
    EXPECT_NO_FATAL_FAILURE({
        p->requestHighlight("task1", "/some/path", "keyword", 0);
    });
}

TEST(HighlightProviderTest, RequestHighlightWithCallbackEmitsSignal)
{
    auto *p = HighlightProvider::instance();
    p->setFetchCallback([](const QString &path, const QString &keyword, int) {
        return QString("highlighted: ") + keyword;
    });

    QSignalSpy spy(p, &HighlightProvider::highlightReady);
    p->requestHighlight("task2", "/some/file.txt", "test", 0);

    // Wait for the worker thread to process
    QEventLoop loop;
    QTimer::singleShot(500, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_GE(spy.count(), 1);
}

TEST(HighlightProviderTest, CancelTaskDoesNotCrash)
{
    auto *p = HighlightProvider::instance();
    EXPECT_NO_FATAL_FAILURE({ p->cancelTask("nonexistent_task"); });
}

TEST(HighlightProviderTest, CachedRequestReturnsImmediately)
{
    auto *p = HighlightProvider::instance();
    p->setFetchCallback([](const QString &, const QString &keyword, int) {
        return QString("result: ") + keyword;
    });

    QSignalSpy spy(p, &HighlightProvider::highlightReady);
    p->requestHighlight("task3", "/cache/test.txt", "kw", 0);

    QEventLoop loop;
    QTimer::singleShot(500, &loop, &QEventLoop::quit);
    loop.exec();

    int firstCount = spy.count();
    // Second request for the same path should hit cache
    p->requestHighlight("task3", "/cache/test.txt", "kw", 0);

    QEventLoop loop2;
    QTimer::singleShot(500, &loop2, &QEventLoop::quit);
    loop2.exec();

    EXPECT_GE(spy.count(), firstCount);
}
