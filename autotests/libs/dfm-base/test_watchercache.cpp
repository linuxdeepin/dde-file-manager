// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_watchercache.cpp
 * @brief Unit tests for WatcherCache (utils/watchercache.cpp)
 *
 * AbstractFileWatcher's default ctor is deleted, so the null-watcher path of
 * cacheWatcher() is exercised directly; the cache/remove/disable APIs are
 * driven through deterministic no-op and round-trip cases.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QSharedPointer>
#include <QSignalSpy>

#include <dfm-base/utils/watchercache.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <QList>
#include <dfm-base/interfaces/abstractfilewatcher.h>

using namespace dfmbase;

TEST(WatcherCacheTest, InstanceReturnsSameReference)
{
    WatcherCache &a = WatcherCache::instance();
    WatcherCache &b = WatcherCache::instance();
    EXPECT_EQ(&a, &b);
}

TEST(WatcherCacheTest, LocalInstanceLifecycleRunsDestructor)
{
    // A stack instance exercises the (otherwise never-invoked) destructor of
    // the heap singleton held by instance().
    EXPECT_NO_FATAL_FAILURE({ WatcherCache wc; (void)wc; });
}

TEST(WatcherCacheTest, CacheWatcherWithNullIsNoop)
{
    QUrl url("file:///tmp/ut_watchercache_null");
    // Null watcher must not be cached: the early-return path skips insert/emit.
    WatcherCache::instance().cacheWatcher(url, QSharedPointer<AbstractFileWatcher>());
    EXPECT_EQ(WatcherCache::instance().getCacheWatcher(url).isNull(), true);
}

TEST(WatcherCacheTest, GetCacheWatcherForAbsentUrlReturnsNull)
{
    QUrl url("file:///tmp/ut_watchercache_absent");
    QSharedPointer<AbstractFileWatcher> w = WatcherCache::instance().getCacheWatcher(url);
    EXPECT_TRUE(w.isNull());
}

TEST(WatcherCacheTest, RemoveCacheWatcherForAbsentUrlEmitsFileDelete)
{
    QUrl url("file:///tmp/ut_watchercache_remove_absent");
    QSignalSpy deleteSpy(&WatcherCache::instance(), &WatcherCache::fileDelete);
    QSignalSpy timeSpy(&WatcherCache::instance(), &WatcherCache::updateWatcherTime);
    ASSERT_TRUE(deleteSpy.isValid());
    ASSERT_TRUE(timeSpy.isValid());
    WatcherCache::instance().removeCacheWatcher(url, true);
    EXPECT_EQ(deleteSpy.count(), 1);
    EXPECT_EQ(timeSpy.count(), 1);
}

TEST(WatcherCacheTest, RemoveCacheWatcherByParentRootPathIsNoop)
{
    QUrl root;
    root.setScheme("file");
    root.setPath("/");
    QSignalSpy timeSpy(&WatcherCache::instance(), &WatcherCache::updateWatcherTime);
    ASSERT_TRUE(timeSpy.isValid());
    WatcherCache::instance().removeCacheWatcherByParent(root);
    // Root path ("/") short-circuits before removing anything.
    EXPECT_EQ(timeSpy.count(), 0);
}

TEST(WatcherCacheTest, SetCacheDisableRoundTrips)
{
    const QString scheme = "ut_scheme_disable";
    // Enable then check, then disable then check.
    WatcherCache::instance().setCacheDisbale(scheme, true);
    EXPECT_TRUE(WatcherCache::instance().cacheDisable(scheme));
    WatcherCache::instance().setCacheDisbale(scheme, false);
    EXPECT_FALSE(WatcherCache::instance().cacheDisable(scheme));
    // Re-enabling an already-disabled scheme is a no-op but stays disabled.
    WatcherCache::instance().setCacheDisbale(scheme, true);
    WatcherCache::instance().setCacheDisbale(scheme, true);
    EXPECT_TRUE(WatcherCache::instance().cacheDisable(scheme));
    // cleanup
    WatcherCache::instance().setCacheDisbale(scheme, false);
}

// ============================================================
// PMS sev-2 regression cluster: watchercache.cpp (work-order batch 3)
// ============================================================

// PMS:230183 拖动文件进回收站桌面崩溃：cacheWatcher 对空 url/空 watcher 安全，
// 有效监听对象入缓存并广播 updateWatcherTime({url}, true)
TEST(WatcherCacheTest, BUG230183_CacheWatcherInsertsAndEmitsUpdateWatcherTime)
{
    const QUrl url = QUrl::fromLocalFile(QString("/tmp/dfm_ut_watchercache_230183.txt"));
    auto watcher = QSharedPointer<LocalFileWatcher>::create(url);
    QSignalSpy spy(&WatcherCache::instance(), &WatcherCache::updateWatcherTime);
    WatcherCache::instance().cacheWatcher(url, watcher);
    EXPECT_EQ(WatcherCache::instance().getCacheWatcher(url), watcher);
    // ctor auto-registration and cache hits may emit extra updateWatcherTime;
    // require at least one emission advertising this url with add=true
    ASSERT_GE(spy.count(), 1);
    bool found = false;
    for (const auto &args : spy) {
        if (args.at(0).value<QList<QUrl>>().contains(url) && args.at(1).toBool()) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
    WatcherCache::instance().removeCacheWatcher(url);
}

// PMS:268167 批量删除文件夹时文件管理器崩溃：removeCacheWatcherByParent 按
// scheme+路径前缀批量清理并广播 updateWatcherTime(urls, false)；根目录直接短路
TEST(WatcherCacheTest, BUG268167_RemoveByParentPrefixEmitsUpdateWatcherTime)
{
    const QUrl parent = QUrl::fromLocalFile(QString("/tmp/dfm_ut_watchercache_268167"));
    const QUrl childA = QUrl::fromLocalFile(parent.path() + "/subA/f1.txt");
    const QUrl childB = QUrl::fromLocalFile(parent.path() + "/subB/f2.txt");
    const QUrl outside = QUrl::fromLocalFile(QString("/tmp/dfm_ut_watchercache_outside_268167.txt"));
    WatcherCache::instance().cacheWatcher(childA, QSharedPointer<LocalFileWatcher>::create(childA));
    WatcherCache::instance().cacheWatcher(childB, QSharedPointer<LocalFileWatcher>::create(childB));
    WatcherCache::instance().cacheWatcher(outside, QSharedPointer<LocalFileWatcher>::create(outside));
    ASSERT_FALSE(WatcherCache::instance().getCacheWatcher(childA).isNull());

    QSignalSpy spy(&WatcherCache::instance(), &WatcherCache::updateWatcherTime);
    const int baseline = spy.count();
    WatcherCache::instance().removeCacheWatcherByParent(parent);
    EXPECT_TRUE(WatcherCache::instance().getCacheWatcher(childA).isNull());
    EXPECT_TRUE(WatcherCache::instance().getCacheWatcher(childB).isNull());
    EXPECT_FALSE(WatcherCache::instance().getCacheWatcher(outside).isNull());
    // emissions advertising the removed children with add=false (per-child emits)
    ASSERT_GT(spy.count(), baseline);
    bool removedA = false, removedB = false;
    for (int i = baseline; i < spy.count(); ++i) {
        const auto removed = spy.at(i).at(0).value<QList<QUrl>>();
        if (!spy.at(i).at(1).toBool()) {
            removedA = removedA || removed.contains(childA);
            removedB = removedB || removed.contains(childB);
        }
    }
    EXPECT_TRUE(removedA);
    EXPECT_TRUE(removedB);

    // root parent short-circuit: no crash, no extra signal
    const int beforeRoot = spy.count();
    WatcherCache::instance().removeCacheWatcherByParent(QUrl::fromLocalFile("/"));
    EXPECT_EQ(spy.count(), beforeRoot);

    WatcherCache::instance().removeCacheWatcher(outside);
}
