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

#include <QMutex>
#include <QTimer>
#include <QMap>

namespace dfmbase {

class TimeSortedUrlList
{
    QMap<QString, QUrl> forSort;
    QMap<QUrl, QString> forQuery;
    QMutex mutex;

public:
    inline int count()
    {
        return forSort.count();
    }
    inline QList<QUrl> list()
    {
        return forSort.values();
    }
    inline bool contains(const QUrl &url)
    {
        return forQuery.contains(url);
    }
    inline void remove(const QUrl &url)
    {
        if (forQuery.contains(url)) {
            auto key = forQuery.value(url);
            forQuery.remove(url);
            forSort.remove(key);
        }
    }
    inline QUrl first()
    {
        return forSort.count() > 0 ? forSort.first() : QUrl();
    }
    inline void pushBack(const QUrl &url)
    {
        if (!forQuery.contains(url)) {
            QString key = QString("%1_%2").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()).arg(url.toString());
            forSort.insert(key, url);
            forQuery.insert(url, key);
        } else {
            remove(url);
            pushBack(url);
        }
    }

    inline void lock() { mutex.lock(); }
    inline void unlock() { mutex.unlock(); }
    inline int lCount()
    {
        QMutexLocker locker(&mutex);
        return count();
    }
    inline QList<QUrl> lList()
    {
        QMutexLocker locker(&mutex);
        return list();
    }
    inline bool lContains(const QUrl &url)
    {
        QMutexLocker lk(&mutex);
        return contains(url);
    }
    inline void lRemove(const QUrl &url)
    {
        QMutexLocker lk(&mutex);
        remove(url);
    }
    inline QUrl lFirst()
    {
        QMutexLocker lk(&mutex);
        return first();
    }
    inline void lPushBack(const QUrl &url)
    {
        QMutexLocker lk(&mutex);
        pushBack(url);
    }
};

class InfoCachePrivate
{
    friend class InfoCache;
    InfoCache *const q;
    DThreadMap<QUrl, AbstractFileInfoPointer> fileInfos;   // 缓存fileifno的Map
    DThreadList<QUrl> needRemoveCacheList;   // 待移除的fileinfo的urllist
    DThreadList<QUrl> removedCacheList;   // 已被removecache的url
    DThreadList<QUrl> removedSortByTimeCacheList;   // 已被SortByTimeCache的url
    QSharedPointer<ReFreshThread> refreshThread { nullptr };   // 刷新线程
    TimeSortedUrlList sortByTimeCacheUrl;   // 按时间排序的缓存fileinfo的文件url
    QTimer needRemoveTimer;   // 需要加入待移除缓存的计时器
    QTimer removeTimer;   // 移除缓存的
    DThreadList<QString> disableCahceSchemes;

public:
    explicit InfoCachePrivate(InfoCache *qq);
    virtual ~InfoCachePrivate();
    void updateSortByTimeCacheUrlList(const QUrl &url);
};
}

#endif   // INFOCACHE_P_H
