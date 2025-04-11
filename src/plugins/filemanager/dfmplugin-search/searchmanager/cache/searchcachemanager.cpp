// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchcachemanager.h"

#include <dfm-base/base/application/application.h>

#include <QCryptographicHash>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

// Singleton implementation
SearchCacheManager *SearchCacheManager::instance()
{
    static SearchCacheManager instance;
    return &instance;
}

SearchCacheManager::SearchCacheManager(QObject *parent)
    : QObject(parent)
{
    // Configure thread pool
    threadPool.setMaxThreadCount(2);  // Limit maximum thread count
}

SearchCacheManager::~SearchCacheManager()
{
}

QString SearchCacheManager::getCacheKey(const QUrl &url, const QString &keyword) const
{
    // Generate unique cache key using URL and keyword
    QString combined = url.toString() + "|" + keyword;
    return QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Md5).toHex();
}

bool SearchCacheManager::isCacheValid(const SearchCache &cache) const
{
    // Check if cache is valid (not expired)
    return cache.timestamp.addSecs(cacheExpireSeconds) > QDateTime::currentDateTime();
}

bool SearchCacheManager::hasCache(const QUrl &url, const QString &keyword)
{
    QString key = getCacheKey(url, keyword);
    
    QReadLocker locker(&cacheLock);
    bool hasValid = searchCaches.contains(key) && isCacheValid(searchCaches[key]);
    
    if (hasValid) {
        fmInfo() << "Found valid cache:" << keyword << "in" << url.toString()
                 << "Cache time:" << searchCaches[key].timestamp.toString()
                 << "Result count:" << searchCaches[key].results.size();
    }
    
    return hasValid;
}

DFMSearchResultMap SearchCacheManager::getResultsFromCache(const QUrl &url, const QString &keyword)
{
    QString key = getCacheKey(url, keyword);
    
    QReadLocker locker(&cacheLock);
    if (searchCaches.contains(key) && isCacheValid(searchCaches[key])) {
        return searchCaches[key].results;
    }
    return {};
}

void SearchCacheManager::updateCache(const QString &taskId, const QUrl &url, const QString &keyword, 
                                     const DFMSearchResultMap &results)
{
    // Only update cache when there are results
    if (results.isEmpty()) {
        fmInfo() << "Search results empty, not updating cache";
        return;
    }
    
    // Submit cache update operation to thread pool
    threadPool.start([this, url, keyword, results]() {
        QString key = getCacheKey(url, keyword);
        SearchCache cache;
        cache.searchUrl = url;
        cache.keyword = keyword;
        cache.results = results;
        cache.timestamp = QDateTime::currentDateTime();
        
        QWriteLocker locker(&cacheLock);
        searchCaches[key] = cache;
        
        fmInfo() << "Updated search cache:" << keyword << "in" << url.toString() 
                 << "Result count:" << results.size()
                 << "Cache time:" << cache.timestamp.toString();
    });
}

void SearchCacheManager::clearSearchCache(const QUrl &url, const QString &keyword)
{
    // Submit cache clearing operation to thread pool
    threadPool.start([this, url, keyword]() {
        QString key = getCacheKey(url, keyword);
        
        QWriteLocker locker(&cacheLock);
        if (searchCaches.contains(key)) {
            searchCaches.remove(key);
            fmInfo() << "Cleared search cache:" << keyword << "in" << url.toString();
        }
    });
}

void SearchCacheManager::clearAllSearchCache()
{
    // Submit all cache clearing operation to thread pool
    threadPool.start([this]() {
        QWriteLocker locker(&cacheLock);
        int count = searchCaches.size();
        searchCaches.clear();
        fmInfo() << "Cleared all search caches, total" << count << "items";
    });
}

void SearchCacheManager::setCacheExpireTime(int seconds)
{
    if (seconds > 0) {
        cacheExpireSeconds = seconds;
        fmInfo() << "Set cache expire time to" << seconds << "seconds";
    }
}
