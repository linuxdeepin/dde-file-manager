// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHCACHEMANAGER_H
#define SEARCHCACHEMANAGER_H

#include "dfmplugin_search_global.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <QObject>
#include <QHash>
#include <QUrl>
#include <QDateTime>
#include <QThreadPool>
#include <QReadWriteLock>

DPSEARCH_BEGIN_NAMESPACE

// 统一搜索缓存结构
struct SearchCache {
    QUrl searchUrl;                 // 搜索路径
    QString keyword;                // 搜索关键字
    DFMSearchResultMap results;        // 使用统一的结果数据结构
    QDateTime timestamp;            // 缓存创建时间
};

class SearchCacheManager : public QObject
{
    Q_OBJECT
public:
    static SearchCacheManager *instance();

    // 检查是否有缓存
    bool hasCache(const QUrl &url, const QString &keyword);
    
    // 从缓存获取结果 - 使用统一的SearchResultMap
    DFMSearchResultMap getResultsFromCache(const QUrl &url, const QString &keyword);
    
    // 更新缓存 - 使用统一的SearchResultMap
    void updateCache(const QString &taskId, const QUrl &url, const QString &keyword, 
                     const DFMSearchResultMap &results);
    
    // 清除特定的搜索缓存
    void clearSearchCache(const QUrl &url, const QString &keyword);
    
    // 清除所有缓存
    void clearAllSearchCache();
    
    // 设置缓存过期时间（秒）
    void setCacheExpireTime(int seconds);

private:
    explicit SearchCacheManager(QObject *parent = nullptr);
    ~SearchCacheManager();
    
    // 获取缓存的key
    QString getCacheKey(const QUrl &url, const QString &keyword) const;
    // 检查缓存是否有效
    bool isCacheValid(const SearchCache &cache) const;

    QHash<QString, SearchCache> searchCaches;      // 搜索结果缓存
    mutable QReadWriteLock cacheLock;              // 保护缓存的读写锁
    QThreadPool threadPool;                        // 线程池用于异步操作
    int cacheExpireSeconds = 300;                  // 缓存过期时间，默认5分钟
};

DPSEARCH_END_NAMESPACE

#endif // SEARCHCACHEMANAGER_H 