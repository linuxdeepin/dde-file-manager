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

#ifndef WATCHERCACHE_H
#define WATCHERCACHE_H

#include "dfm-base/base/abstractfilewatcher.h"

#include <QObject>
#include <QUrl>

DFMBASE_BEGIN_NAMESPACE
class WacherFactory;
class InfoCache;
class WatcherCachePrivate;
class WatcherCache : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), WatcherCache)
    QScopedPointer<WatcherCachePrivate> d_ptr;
    friend WacherFactory;
    friend InfoCache;
public:
    explicit WatcherCache(QObject *parent = nullptr);
    virtual ~WatcherCache();
    static WatcherCache& instance();
    QSharedPointer<AbstractFileWatcher> getCacheWatcher(const QUrl &url);
    void cacheWatcher(const QUrl &url, const QSharedPointer<AbstractFileWatcher> &watcher);
    void removCacheWatcher(const QUrl &url);
};
DFMBASE_END_NAMESPACE

#endif // WATCHERCACHE_H
