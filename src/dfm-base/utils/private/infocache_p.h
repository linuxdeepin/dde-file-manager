// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INFOCACHE_P_H
#define INFOCACHE_P_H

#include <dfm-base/utils/infocache.h>

#include <QReadWriteLock>
#include <QMutex>
#include <QTimer>
#include <QMap>

namespace dfmbase {
enum CacheInfoStatus : uint8_t {
    kCacheMain = 0,   // 1.正常状态 插入(同时插入主和副缓存hash)， 读取主缓存hash， 删除主缓存hash
    kCacheCopy,
};
class InfoCachePrivate
{
    friend class InfoCache;
    friend class CacheWorker;
    InfoCache *const q;
    DThreadList<QString> disableCahceSchemes;

    QAtomicInt status { kCacheMain };   // 当前状态缓存的状态
    QHash<QUrl, FileInfoPointer> mainCache;   // 主信息缓存
    QHash<QUrl, FileInfoPointer> copyCache;   // 副信息缓存
    QReadWriteLock mianLock;
    QReadWriteLock copyLock;

    // 时间排序url,利用map的有序性，来处理时间到了要移除的url
    QHash<QUrl, QString> urlTimeSortHash;
    QMap<QString, QUrl> timeToUrlMap;

    // 时间排序url,利用map的有序性，来处理时间到了要移除的url
    QHash<QUrl, QString> urlTimeSortWatcherHash;
    QMap<QString, QUrl> timeToUrlWatcherMap;
    std::atomic_bool cacheWorkerStoped { false };

public:
    explicit InfoCachePrivate(InfoCache *qq);
    virtual ~InfoCachePrivate();
};
}

#endif   // INFOCACHE_P_H
