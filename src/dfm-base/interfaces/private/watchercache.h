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
#ifndef WATCHERCACHE_H
#define WATCHERCACHE_H

#include "dfm-base/interfaces/abstractfilewatcher.h"

#include <QObject>
#include <QUrl>

namespace dfmbase {
class WatcherFactory;
class InfoCache;
class WatcherCachePrivate;
class WatcherCache : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d), WatcherCache)
    QScopedPointer<WatcherCachePrivate> d;
    friend WatcherFactory;
    friend InfoCache;

public:
    static WatcherCache &instance();
    explicit WatcherCache(QObject *parent = nullptr);
    virtual ~WatcherCache();
    QSharedPointer<AbstractFileWatcher> getCacheWatcher(const QUrl &url);
    void cacheWatcher(const QUrl &url, const QSharedPointer<AbstractFileWatcher> &watcher);
    void removeCacheWatcher(const QUrl &url);
    bool cacheDisable(const QString &scheme);
    void setCacheDisbale(const QString &scheme, bool disbale = true);
};
}

#endif   // WATCHERCACHE_H
