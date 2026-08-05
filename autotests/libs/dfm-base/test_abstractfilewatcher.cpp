// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractfilewatcher.cpp
 * @brief Unit tests for AbstractFileWatcher default implementations.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QDir>
#include <QTemporaryDir>

#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>
#include <dfm-base/base/urlroute.h>

using namespace dfmbase;

class TestWatcherPrivate : public AbstractFileWatcherPrivate
{
public:
    explicit TestWatcherPrivate(const QUrl &url, AbstractFileWatcher *q)
        : AbstractFileWatcherPrivate(url, q) { }

    bool start() override
    {
        return true;
    }
    bool stop() override
    {
        return true;
    }
};

class TestFileWatcher : public AbstractFileWatcher
{
public:
    explicit TestFileWatcher(const QUrl &url, QObject *parent = nullptr)
        : AbstractFileWatcher(new TestWatcherPrivate(url, this), parent) { }
};

class AbstractFileWatcherTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
    }

    QTemporaryDir tmpDir;
    QString rootPath;
};

TEST_F(AbstractFileWatcherTest, UrlReturnsWatchedUrl)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    TestFileWatcher watcher(url);
    EXPECT_EQ(watcher.url().scheme(), url.scheme());
}

TEST_F(AbstractFileWatcherTest, StartStopRestart)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    TestFileWatcher watcher(url);
    EXPECT_TRUE(watcher.startWatcher());
    EXPECT_TRUE(watcher.stopWatcher());
    // restartWatcher = stopWatcher() && startWatcher(); both true here
    EXPECT_TRUE(watcher.restartWatcher());
}

TEST_F(AbstractFileWatcherTest, StopWhenNotStartedReturnsTrue)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    TestFileWatcher watcher(url);
    EXPECT_TRUE(watcher.stopWatcher());
}

TEST_F(AbstractFileWatcherTest, StartIdempotent)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    TestFileWatcher watcher(url);
    EXPECT_TRUE(watcher.startWatcher());
    EXPECT_TRUE(watcher.startWatcher());   // already started
}

TEST_F(AbstractFileWatcherTest, SetEnabledSubfileWatcherNoCrash)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    TestFileWatcher watcher(url);
    EXPECT_NO_FATAL_FAILURE({ watcher.setEnabledSubfileWatcher(QUrl("file:///tmp/sub"), true); });
}

TEST_F(AbstractFileWatcherTest, CacheInfoConnectSizeManipulation)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    TestFileWatcher watcher(url);
    EXPECT_EQ(watcher.getCacheInfoConnectSize(), 0);
    watcher.addCacheInfoConnectSize();
    EXPECT_EQ(watcher.getCacheInfoConnectSize(), 1);
    watcher.reduceCacheInfoConnectSize();
    EXPECT_EQ(watcher.getCacheInfoConnectSize(), 0);
}

TEST_F(AbstractFileWatcherTest, NotifyMethodsNoCrash)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    TestFileWatcher watcher(url);
    EXPECT_NO_FATAL_FAILURE({ watcher.notifyFileAdded(QUrl("file:///tmp/new")); });
    EXPECT_NO_FATAL_FAILURE({ watcher.notifyFileChanged(QUrl("file:///tmp/changed")); });
    EXPECT_NO_FATAL_FAILURE({ watcher.notifyFileDeleted(QUrl("file:///tmp/gone")); });
}

TEST_F(AbstractFileWatcherTest, FormatPathStatic)
{
    QString p = AbstractFileWatcherPrivate::formatPath("/tmp/some_path/");
    EXPECT_FALSE(p.endsWith("/"));
    EXPECT_EQ(p, QString("/tmp/some_path"));
}
