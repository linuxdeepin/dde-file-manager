/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef INFOCACHE_P_H
#define INFOCACHE_P_H

#include "infocache.h"

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
    friend class Worker;
    InfoCache *const q;
    DThreadList<QString> disableCahceSchemes;

    QAtomicInt status { kCacheMain };   // 当前状态缓存的状态
    QHash<QUrl, AbstractFileInfoPointer> mainCache;   // 主信息缓存
    QHash<QUrl, AbstractFileInfoPointer> copyCache;   // 副信息缓存
    QReadWriteLock mianLock;
    QReadWriteLock copyLock;

    // 时间排序url,利用map的有序性，来处理时间到了要移除的url
    QMap<QUrl, QString> urlTimeSortMap;
    QMap<QString, QUrl> timeToUrlMap;

    std::atomic_bool cacheWorkerStoped { false };

public:
    explicit InfoCachePrivate(InfoCache *qq);
    virtual ~InfoCachePrivate();
};
}

#endif   // INFOCACHE_P_H
