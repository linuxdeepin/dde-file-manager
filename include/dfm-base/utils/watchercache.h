// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WATCHERCACHE_H
#define WATCHERCACHE_H

#include <dfm-base/interfaces/abstractfilewatcher.h>

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
Q_SIGNALS:
    void fileDelete(const QUrl &url);
    void updateWatcherTime(const QList<QUrl> &url, const bool add);

public:
    static WatcherCache &instance();
    explicit WatcherCache(QObject *parent = nullptr);
    virtual ~WatcherCache();
    QSharedPointer<AbstractFileWatcher> getCacheWatcher(const QUrl &url);

    void cacheWatcher(const QUrl &url, const QSharedPointer<AbstractFileWatcher> &watcher);
    void removeCacheWatcher(const QUrl &url, const bool isEmit = true);
    void removeCacheWatcherByParent(const QUrl &parent);
    bool cacheDisable(const QString &scheme);
    void setCacheDisbale(const QString &scheme, bool disbale = true);
};
}   // namespace dfmbase

#endif   // WATCHERCACHE_H
