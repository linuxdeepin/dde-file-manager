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

#include "base/private/dfmwatchercachesmanager.h"
#include "base/private/dfmwatchercachesmanager_p.h"
#include <QSharedPointer>

Q_GLOBAL_STATIC(DFMWatcherCachesManager,_watcherCacheManager)

DFMWatcherCachesManagerPrivate::DFMWatcherCachesManagerPrivate(DFMWatcherCachesManager *qq)
    : q_ptr(qq)
{

}

DFMWatcherCachesManager::DFMWatcherCachesManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMWatcherCachesManagerPrivate(this))
{

}

DFMWatcherCachesManager::~DFMWatcherCachesManager()
{

}

DFMWatcherCachesManager &DFMWatcherCachesManager::instance()
{
    return *_watcherCacheManager;
}

QSharedPointer<DAbstractFileWatcher> DFMWatcherCachesManager::getCacheWatcher(const QUrl &url)
{
    Q_D(DFMWatcherCachesManager);

    return d->m_watchers.value(url);
}

void DFMWatcherCachesManager::cacheWatcher(const QUrl &url, const QSharedPointer<DAbstractFileWatcher> &watcher)
{
    Q_D(DFMWatcherCachesManager);

    d->m_watchers.insert(url,watcher);
}

void DFMWatcherCachesManager::removCacheWatcher(const QUrl &url)
{
    Q_D(DFMWatcherCachesManager);

    d->m_watchers.remove(url);
}
