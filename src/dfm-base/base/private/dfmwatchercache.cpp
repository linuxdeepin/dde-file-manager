/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "base/private/dfmwatchercache.h"
#include "base/private/dfmwatchercache_p.h"

#include <QSharedPointer>

Q_GLOBAL_STATIC(DFMWatcherCache,_watcherCacheManager)

DFMWatcherCachePrivate::DFMWatcherCachePrivate(DFMWatcherCache *qq)
    : q_ptr(qq)
{

}

DFMWatcherCache::DFMWatcherCache(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMWatcherCachePrivate(this))
{

}

DFMWatcherCache::~DFMWatcherCache()
{

}

DFMWatcherCache &DFMWatcherCache::instance()
{
    return *_watcherCacheManager;
}

QSharedPointer<DAbstractFileWatcher> DFMWatcherCache::getCacheWatcher(const QUrl &url)
{
    Q_D(DFMWatcherCache);

    return d->m_watchers.value(url);
}

void DFMWatcherCache::cacheWatcher(const QUrl &url, const QSharedPointer<DAbstractFileWatcher> &watcher)
{
    Q_D(DFMWatcherCache);

    d->m_watchers.insert(url,watcher);
}

void DFMWatcherCache::removCacheWatcher(const QUrl &url)
{
    Q_D(DFMWatcherCache);

    d->m_watchers.remove(url);
}
