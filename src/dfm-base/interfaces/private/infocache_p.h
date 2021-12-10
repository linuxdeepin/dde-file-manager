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

DFMBASE_BEGIN_NAMESPACE
class InfoCachePrivate
{
    friend class InfoCache;
    InfoCache *const q;
    DThreadMap<QUrl, AbstractFileInfoPointer> fileInfos;   // 缓存fileifno的Map
    DThreadList<QUrl> needRemoveCacheList;   // 待移除的fileinfo的urllist
    DThreadList<QUrl> removedCacheList;   // 已被removecache的url
    DThreadList<QUrl> removedSortByTimeCacheList;   // 已被SortByTimeCache的url
    QSharedPointer<ReFreshThread> refreshThread { nullptr };   // 刷新线程
    DThreadList<QUrl> sortByTimeCacheUrl;   // 按时间排序的缓存fileinfo的文件url
    QTimer needRemoveTimer;   // 需要加入待移除缓存的计时器
    QTimer removeTimer;   // 移除缓存的
public:
    explicit InfoCachePrivate(InfoCache *qq);
    virtual ~InfoCachePrivate();
    void updateSortByTimeCacheUrlList(const QUrl &url);
};
DFMBASE_END_NAMESPACE

#endif   // INFOCACHE_P_H
