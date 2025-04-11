// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchcachemanager.h"

#include <dfm-base/base/application/application.h>

#include <QCryptographicHash>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

// 单例实现
SearchCacheManager *SearchCacheManager::instance()
{
    static SearchCacheManager instance;
    return &instance;
}

SearchCacheManager::SearchCacheManager(QObject *parent)
    : QObject(parent)
{
    // 配置线程池
    threadPool.setMaxThreadCount(2);  // 限制最大线程数
}

SearchCacheManager::~SearchCacheManager()
{
}

QString SearchCacheManager::getCacheKey(const QUrl &url, const QString &keyword) const
{
    // 使用URL和关键字生成唯一缓存键
    QString combined = url.toString() + "|" + keyword;
    return QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Md5).toHex();
}

bool SearchCacheManager::isCacheValid(const SearchCache &cache) const
{
    // 检查缓存是否有效（是否过期）
    return cache.timestamp.addSecs(cacheExpireSeconds) > QDateTime::currentDateTime();
}

bool SearchCacheManager::hasCache(const QUrl &url, const QString &keyword)
{
    QString key = getCacheKey(url, keyword);
    
    QReadLocker locker(&cacheLock);
    bool hasValid = searchCaches.contains(key) && isCacheValid(searchCaches[key]);
    
    if (hasValid) {
        fmInfo() << "找到有效缓存:" << keyword << "in" << url.toString()
                 << "缓存时间:" << searchCaches[key].timestamp.toString()
                 << "结果数量:" << searchCaches[key].results.size();
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
    // 只有在有结果时才更新缓存
    if (results.isEmpty()) {
        fmInfo() << "搜索结果为空，不更新缓存";
        return;
    }
    
    // 将缓存更新操作放入线程池
    QtConcurrent::run(&threadPool, [this, url, keyword, results]() {
        QString key = getCacheKey(url, keyword);
        SearchCache cache;
        cache.searchUrl = url;
        cache.keyword = keyword;
        cache.results = results;
        cache.timestamp = QDateTime::currentDateTime();
        
        QWriteLocker locker(&cacheLock);
        searchCaches[key] = cache;
        
        fmInfo() << "更新搜索缓存:" << keyword << "in" << url.toString() 
                 << "结果数量:" << results.size()
                 << "缓存时间:" << cache.timestamp.toString();
    });
}

void SearchCacheManager::clearSearchCache(const QUrl &url, const QString &keyword)
{
    // 将缓存清除操作放入线程池
    QtConcurrent::run(&threadPool, [this, url, keyword]() {
        QString key = getCacheKey(url, keyword);
        
        QWriteLocker locker(&cacheLock);
        if (searchCaches.contains(key)) {
            searchCaches.remove(key);
            fmInfo() << "清除搜索缓存:" << keyword << "in" << url.toString();
        }
    });
}

void SearchCacheManager::clearAllSearchCache()
{
    // 将全部缓存清除操作放入线程池
    QtConcurrent::run(&threadPool, [this]() {
        QWriteLocker locker(&cacheLock);
        int count = searchCaches.size();
        searchCaches.clear();
        fmInfo() << "清除所有搜索缓存, 共" << count << "项";
    });
}

void SearchCacheManager::setCacheExpireTime(int seconds)
{
    if (seconds > 0) {
        cacheExpireSeconds = seconds;
        fmInfo() << "设置缓存过期时间为" << seconds << "秒";
    }
}
