// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/watchercache_p.h"

#include <QSharedPointer>

namespace dfmbase {
Q_GLOBAL_STATIC(WatcherCache, _watcherCacheManager)

/*!
 * \class DFMWatcherCachesManager
 *
 * \brief 这个类是对Url的强对应，必须保证url的唯一性
 */
WatcherCachePrivate::WatcherCachePrivate(WatcherCache *qq)
    : q(qq)
{
}

WatcherCachePrivate::~WatcherCachePrivate()
{
}

WatcherCache::WatcherCache(QObject *parent)
    : QObject(parent), d(new WatcherCachePrivate(this))
{
}

WatcherCache::~WatcherCache()
{
}
/*!
 * \brief  instance 获取全局的文件监视器的缓存管理模块实例
 *
 *  只有提供给自己使用DFMWatcherFactory，不对外做操作防止出现多个watcher
 *
 * \return DWatcherCachesManager 返回全局的文件监视器的缓存管理模块实例
 */
WatcherCache &WatcherCache::instance()
{
    return *_watcherCacheManager;
}
/*!
 * \brief getCacheWatcher 根据url获取缓存中对应的监视器
 *
 * 只有提供给自己使用DFMWatcherFactory，不对外做操作防止出现多个watcher
 *
 * \param const QUrl &url 需要构造的Url
 *
 * \return QSharedPointer<T> 返回相应的缓存的watcher，如果没有
 *
 * 就返回空指针
 */
QSharedPointer<AbstractFileWatcher> WatcherCache::getCacheWatcher(const QUrl &url)
{
    Q_D(WatcherCache);
    emit updateWatcherTime({url}, true);
    return d->watchers.value(url);
}
/*!
 * \brief cacheWatcher 缓存url对应的watcher
 *
 *  只有提供给自己使用DFMWatcherFactory，不对外做操作防止出现多个watcher
 *
 *  watcher必须有值，如果为nullptr那就不会缓存
 *
 * \param const QUrl &url 需要构造的Url
 *
 * \param QSharedPointer<DAbstractFileWatcher> &watcher 缓存的watcher
 *
 * \return void
 */
void WatcherCache::cacheWatcher(const QUrl &url, const QSharedPointer<AbstractFileWatcher> &watcher)
{
    Q_D(WatcherCache);
    if (watcher.isNull())
        return;
    connect(watcher.data(), &AbstractFileWatcher::fileDeleted, this, &WatcherCache::fileDelete);
    d->watchers.insert(url, watcher);
    emit updateWatcherTime({url}, true);
}
/*!
 * \brief removCacheWatcher 根据Url移除当前缓存的watcher
 *
 *  只有提供给自己使用DFMWatcherFactory，不对外做操作防止出现多个watcher
 *
 *  首先需要注册scheme到DFMUrlRoute类
 *
 *  其次需要注册scheme到DFMSchemeFactory<T>类
 *
 * \param const QUrl &url 需要移除的url
 *
 * \return void
 */
void WatcherCache::removeCacheWatcher(const QUrl &url, const bool isEmit)
{
    Q_D(WatcherCache);
    emit fileDelete(url);
    d->watchers.remove(url);
    if (isEmit)
        emit updateWatcherTime({url}, false);
}

void WatcherCache::removeCacheWatcherByParent(const QUrl &parent)
{
    if (parent.path() == "/")
        return;

    Q_D(WatcherCache);
    auto keys = d->watchers.keys();
    QList<QUrl> removeUrls;
    for (const auto &url : keys) {
        if (url.scheme() == parent.scheme() && url.path().startsWith(parent.path())) {
            d->watchers.remove(url);
            removeUrls.append(url);
        }
    }

    emit updateWatcherTime(removeUrls, false);
}

bool WatcherCache::cacheDisable(const QString &scheme)
{
    return d->disableCahceSchemes.contains(scheme);
}

void WatcherCache::setCacheDisbale(const QString &scheme, bool disbale)
{
    if (!d->disableCahceSchemes.contains(scheme) && disbale) {
        d->disableCahceSchemes.push_backByLock(scheme);
        return;
    }
    if (d->disableCahceSchemes.contains(scheme) && !disbale) {
        d->disableCahceSchemes.removeOneByLock(scheme);
        return;
    }
}
}
