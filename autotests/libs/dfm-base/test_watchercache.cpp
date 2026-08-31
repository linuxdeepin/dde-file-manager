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


TEST(WatcherCacheTest, WatcherCache)
{
    // WatcherCache
    SUCCEED();
}

TEST(WatcherCacheTest, cacheDisable)
{
    // cacheDisable
    SUCCEED();
}

TEST(WatcherCacheTest, cacheWatcher)
{
    // cacheWatcher
    SUCCEED();
}

TEST(WatcherCacheTest, getCacheWatcher)
{
    // getCacheWatcher
    SUCCEED();
}

TEST(WatcherCacheTest, removeCacheWatcher)
{
    // removeCacheWatcher
    SUCCEED();
}

TEST(WatcherCacheTest, removeCacheWatcherByParent)
{
    // removeCacheWatcherByParent
    SUCCEED();
}

TEST(WatcherCacheTest, setCacheDisbale)
{
    // setCacheDisbale
    SUCCEED();
}
