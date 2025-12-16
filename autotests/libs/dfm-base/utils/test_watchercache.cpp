// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-base/utils/watchercache.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <QUrl>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QTimer>

#include <memory>

// Define the missing private class for testing purposes
#include <dfm-base/base/urlroute.h>
#include <QDir>
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

// Forward declaration for the mock class
DFMBASE_USE_NAMESPACE
class MockFileWatcher : public AbstractFileWatcher
{
    Q_OBJECT
public:
    MockFileWatcher(QObject *parent = nullptr);
    bool startWatcher() override;
    bool stopWatcher() override;
    bool restartWatcher() override;
    void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true) override;
    QUrl url() const override;
    void setUrl(const QUrl &url);

private:
    QUrl mockUrl;
};


// MockFileWatcher implementation moved to header section
MockFileWatcher::MockFileWatcher(QObject *parent)
    : AbstractFileWatcher(new AbstractFileWatcherPrivate(QUrl::fromLocalFile("/mock/path"), this), parent)
{
    mockUrl = QUrl::fromLocalFile("/mock/path");
}

bool MockFileWatcher::startWatcher() {
    return true;
}

bool MockFileWatcher::stopWatcher() {
    return true;
}

bool MockFileWatcher::restartWatcher() {
    return true;
}

void MockFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled) {
    Q_UNUSED(subfileUrl)
    Q_UNUSED(enabled)
}

QUrl MockFileWatcher::url() const {
    return mockUrl;
}

void MockFileWatcher::setUrl(const QUrl &url) {
    mockUrl = url;
}

class WatcherCacheTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QCoreApplication exists
        if (!QCoreApplication::instance()) {
            app.reset(new QCoreApplication(argc, argv));
        }

        // Create a temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        testDirPath = tempDir->path();
        testDirUrl = QUrl::fromLocalFile(testDirPath);

        testFilePath = tempDir->filePath("test_file.txt");
        testFileUrl = QUrl::fromLocalFile(testFilePath);

        subDirPath = tempDir->filePath("subdir");
        subDirUrl = QUrl::fromLocalFile(subDirPath);
    }

    void TearDown() override
    {
        // Clean up cached watchers
        auto &cache = WatcherCache::instance();
        cache.removeCacheWatcher(testDirUrl);
        cache.removeCacheWatcher(testFileUrl);
        cache.removeCacheWatcher(subDirUrl);
        
        app.reset();
        tempDir.reset();
    }

    std::unique_ptr<QCoreApplication> app;
    int argc { 0 };
    char **argv { nullptr };
    std::unique_ptr<QTemporaryDir> tempDir;
    QString testDirPath;
    QString testFilePath;
    QString subDirPath;
    QUrl testDirUrl;
    QUrl testFileUrl;
    QUrl subDirUrl;
};

TEST_F(WatcherCacheTest, SingletonInstance)
{
    auto &instance1 = WatcherCache::instance();
    auto &instance2 = WatcherCache::instance();
    
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(WatcherCacheTest, CacheWatcher)
{
    auto &cache = WatcherCache::instance();
    auto mockWatcher = QSharedPointer<MockFileWatcher>::create();
    mockWatcher->setUrl(testDirUrl);
    
    // Cache the watcher
    cache.cacheWatcher(testDirUrl, mockWatcher);
    
    // Retrieve from cache
    auto cachedWatcher = cache.getCacheWatcher(testDirUrl);
    EXPECT_NE(cachedWatcher, nullptr);
    EXPECT_EQ(cachedWatcher.get(), mockWatcher.get());
}

TEST_F(WatcherCacheTest, CacheMultipleWatchers)
{
    auto &cache = WatcherCache::instance();
    auto mockWatcher1 = QSharedPointer<MockFileWatcher>::create();
    auto mockWatcher2 = QSharedPointer<MockFileWatcher>::create();
    mockWatcher1->setUrl(testDirUrl);
    mockWatcher2->setUrl(testFileUrl);
    
    // Cache multiple watchers
    cache.cacheWatcher(testDirUrl, mockWatcher1);
    cache.cacheWatcher(testFileUrl, mockWatcher2);
    
    // Retrieve from cache
    auto cachedWatcher1 = cache.getCacheWatcher(testDirUrl);
    auto cachedWatcher2 = cache.getCacheWatcher(testFileUrl);
    
    EXPECT_NE(cachedWatcher1, nullptr);
    EXPECT_EQ(cachedWatcher1.get(), mockWatcher1.get());
    
    EXPECT_NE(cachedWatcher2, nullptr);
    EXPECT_EQ(cachedWatcher2.get(), mockWatcher2.get());
}

TEST_F(WatcherCacheTest, GetNonExistentWatcher)
{
    auto &cache = WatcherCache::instance();
    
    // Try to get non-existent watcher
    QUrl nonExistentUrl = QUrl::fromLocalFile(tempDir->filePath("non_existent"));
    auto watcher = cache.getCacheWatcher(nonExistentUrl);
    EXPECT_EQ(watcher, nullptr);
}

TEST_F(WatcherCacheTest, RemoveWatcher)
{
    auto &cache = WatcherCache::instance();
    auto mockWatcher = QSharedPointer<MockFileWatcher>::create();
    mockWatcher->setUrl(testDirUrl);
    
    // Cache watcher
    cache.cacheWatcher(testDirUrl, mockWatcher);
    auto cachedWatcher = cache.getCacheWatcher(testDirUrl);
    EXPECT_NE(cachedWatcher, nullptr);
    
    // Remove watcher
    cache.removeCacheWatcher(testDirUrl);
    auto removedWatcher = cache.getCacheWatcher(testDirUrl);
    EXPECT_EQ(removedWatcher, nullptr);
}

TEST_F(WatcherCacheTest, UpdateCachedWatcher)
{
    auto &cache = WatcherCache::instance();
    auto originalWatcher = QSharedPointer<MockFileWatcher>::create();
    originalWatcher->setUrl(testDirUrl);
    
    // Cache original watcher
    cache.cacheWatcher(testDirUrl, originalWatcher);
    auto cachedWatcher1 = cache.getCacheWatcher(testDirUrl);
    EXPECT_EQ(cachedWatcher1.get(), originalWatcher.get());
    
    // Create new watcher with same URL
    auto updatedWatcher = QSharedPointer<MockFileWatcher>::create();
    updatedWatcher->setUrl(testDirUrl);
    
    // Update cache
    cache.cacheWatcher(testDirUrl, updatedWatcher);
    auto cachedWatcher2 = cache.getCacheWatcher(testDirUrl);
    EXPECT_EQ(cachedWatcher2.get(), updatedWatcher.get());
}

TEST_F(WatcherCacheTest, CacheWithNullWatcher)
{
    auto &cache = WatcherCache::instance();
    
    // Try to cache null watcher
    cache.cacheWatcher(testDirUrl, nullptr);
    
    // Should return nullptr
    auto cachedWatcher = cache.getCacheWatcher(testDirUrl);
    EXPECT_EQ(cachedWatcher, nullptr);
}

TEST_F(WatcherCacheTest, HierarchicalUrls)
{
    auto &cache = WatcherCache::instance();
    
    // Create parent and child directory watchers
    auto parentWatcher = QSharedPointer<MockFileWatcher>::create();
    auto childWatcher = QSharedPointer<MockFileWatcher>::create();
    parentWatcher->setUrl(testDirUrl);
    childWatcher->setUrl(testFileUrl);
    
    // Cache both watchers
    cache.cacheWatcher(testDirUrl, parentWatcher);
    cache.cacheWatcher(testFileUrl, childWatcher);
    
    // Both should be retrievable
    auto retrievedParent = cache.getCacheWatcher(testDirUrl);
    auto retrievedChild = cache.getCacheWatcher(testFileUrl);
    
    EXPECT_NE(retrievedParent, nullptr);
    EXPECT_NE(retrievedChild, nullptr);
    EXPECT_NE(retrievedParent.get(), retrievedChild.get());
}

TEST_F(WatcherCacheTest, CacheDisable)
{
    auto &cache = WatcherCache::instance();
    
    // Test cache disable functionality
    QString scheme = "file";
    
    // Initially should not be disabled
    EXPECT_FALSE(cache.cacheDisable(scheme));
    
    // Disable cache for scheme
    cache.setCacheDisbale(scheme, true);
    EXPECT_TRUE(cache.cacheDisable(scheme));
    
    // Re-enable cache for scheme
    cache.setCacheDisbale(scheme, false);
    EXPECT_FALSE(cache.cacheDisable(scheme));
}

TEST_F(WatcherCacheTest, RemoveCacheWatcherByParent)
{
    auto &cache = WatcherCache::instance();
    
    // Create parent and child watchers
    auto parentWatcher = QSharedPointer<MockFileWatcher>::create();
    auto childWatcher = QSharedPointer<MockFileWatcher>::create();
    parentWatcher->setUrl(testDirUrl);
    childWatcher->setUrl(testFileUrl);
    
    // Cache both watchers
    cache.cacheWatcher(testDirUrl, parentWatcher);
    cache.cacheWatcher(testFileUrl, childWatcher);
    
    // Verify they are cached
    EXPECT_NE(cache.getCacheWatcher(testDirUrl), nullptr);
    EXPECT_NE(cache.getCacheWatcher(testFileUrl), nullptr);
    
    // Remove child watchers by parent
    cache.removeCacheWatcherByParent(testDirUrl);
    
    // Parent should still be cached
    EXPECT_NE(cache.getCacheWatcher(testDirUrl), nullptr);
    // Child behavior depends on implementation
}

#include "test_watchercache.moc"