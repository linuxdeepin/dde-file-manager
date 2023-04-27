// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/infocache_p.h"
#include <dfm-base/base/schemefactory.h>

#include <dfm-io/dfileinfo.h>

#include <QtConcurrent>

// cache file total count
static constexpr int kCacheFileinfoCount = 20000;
// rotation training time
static constexpr int kRotationTrainingTime = (60 * 1000);
// remove cache time limit
static constexpr int kCacheRemoveTime = (60 * (60 * 1000));

namespace dfmbase {
InfoCachePrivate::InfoCachePrivate(InfoCache *qq)
    : q(qq)
{
}

InfoCachePrivate::~InfoCachePrivate()
{
    cacheWorkerStoped = true;
}

InfoCache::InfoCache(QObject *parent)
    : QObject(parent), d(new InfoCachePrivate(this))
{
}

InfoCache &InfoCache::instance()
{
    static InfoCache cache;
    return cache;
}

InfoCache::~InfoCache()
{
}

/*!
 * \brief disconnectWatcherThread 移除的url断开监视器
 *
 * \param QStringList 文件的URL
 *
 * \return
 */
void InfoCache::disconnectWatcher(const QMap<QUrl, FileInfoPointer> infos)
{
    if (d->cacheWorkerStoped)
        return;
    for (const auto &info : infos) {
        if (!info)
            continue;
        // 断开信号连接
        auto url = UrlRoute::urlParent(info->urlOf(UrlInfoType::kUrl));
        QSharedPointer<AbstractFileWatcher> watcher =
                WatcherCache::instance().getCacheWatcher(url);
        if (watcher) {
            watcher->reduceCacheInfoConnectSize();
            if (watcher->getCacheInfoConnectSize() <= 0) {
                disconnect(watcher.data(), &AbstractFileWatcher::fileDeleted, this,
                           &InfoCache::removeCache);
                disconnect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this,
                           &InfoCache::refreshFileInfo);
                disconnect(watcher.data(), &AbstractFileWatcher::fileRename, this,
                           &InfoCache::removeCache);
                disconnect(watcher.data(), &AbstractFileWatcher::subfileCreated, this,
                           &InfoCache::refreshFileInfo);
                WatcherCache::instance().removeCacheWatcher(url);
            }
        }
    }
}

void InfoCache::removeCache(const QUrl url)
{
    emit cacheRemoveCaches(QList<QUrl>() << url);
}
/*!
 * \brief cacheDisable 当前的scheme是否可以缓存
 *
 * \param QString 文件的scheme
 *
 * \return
 */
bool InfoCache::cacheDisable(const QString &scheme)
{
    return d->disableCahceSchemes.contains(scheme);
}
/*!
 * \brief setCacheDisbale 设置scheme是否可以缓存
 *
 * \param QString 文件的scheme
 *
 * \param bool 是否可以缓存
 *
 * \return
 */
void InfoCache::setCacheDisbale(const QString &scheme, bool disable)
{
    if (!d->disableCahceSchemes.contains(scheme) && disable) {
        d->disableCahceSchemes.push_backByLock(scheme);
        return;
    }
    if (d->disableCahceSchemes.contains(scheme) && !disable) {
        d->disableCahceSchemes.removeOneByLock(scheme);
        return;
    }
}
/*!
 * \brief cacheInfo 缓存fileinfo
 *
 * \param QString 文件的url
 *
 * \param DAbstractFileInfoPointer fileinfo的智能指针
 *
 * \return
 */
void InfoCache::cacheInfo(const QUrl url, const FileInfoPointer info)
{
    Q_D(InfoCache);
    if (!info || d->cacheWorkerStoped)
        return;

    {
        QReadLocker rlk(&d->mianLock);
        if (d->mainCache.contains(url))
            return;
    }

    {
        QReadLocker rlk(&d->copyLock);
        if (d->copyCache.contains(url))
            return;
    }

    //获取监视器，监听当前的file的改变
    QSharedPointer<AbstractFileWatcher> watcher { nullptr };
    watcher = WatcherFactory::create<AbstractFileWatcher>(UrlRoute::urlParent(url));
    if (watcher) {
        if (watcher->getCacheInfoConnectSize() == 0) {
            connect(watcher.data(), &AbstractFileWatcher::fileDeleted, this, &InfoCache::removeCache);
            connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this,
                    &InfoCache::refreshFileInfo);
            connect(watcher.data(), &AbstractFileWatcher::fileRename, this,
                    &InfoCache::removeCache);
            connect(watcher.data(), &AbstractFileWatcher::subfileCreated, this,
                    &InfoCache::refreshFileInfo);
        }
        watcher->addCacheInfoConnectSize();
    }

    // 插入到主和副的所有缓存中
    d->status = kCacheCopy;
    {
        QWriteLocker wlk(&d->mianLock);
        d->mainCache.insert(url, info);
    }
    d->status = kCacheMain;
    {
        QWriteLocker wlk(&d->copyLock);
        d->copyCache.insert(url, info);
    }
    // 使用线程处理加入时间序列问题
    emit cacheUpdateInfoTime(url);
}
/*!
 * \brief insertSortTime 处理当前文件的移除时间
 *
 * \param qint64 文件info的当前时间
 *
 * \param QString 文件的url
 *
 * \return
 */
void InfoCache::updateSortTimeWorker(const QUrl url)
{
    Q_D(InfoCache);
    if (d->cacheWorkerStoped)
        return;
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto key = QString::number(time) + QString("-") + url.toString();
    if (d->urlTimeSortMap.contains(url))
        d->timeToUrlMap.remove(d->urlTimeSortMap.value(url));
    d->timeToUrlMap.insert(key, url);
    d->urlTimeSortMap.insert(url, key);
}

void InfoCache::stop()
{
    Q_D(InfoCache);
    d->cacheWorkerStoped = true;
}
/*!
 * \brief removeCaches 移除缓存的线程执行函数
 *
 * \param QStringList key需要移除的缓存的key
 *
 * \param std::atomic_bool isStop判断当前是否是退出当前处理
 *
 * \return
 */
void InfoCache::removeCaches(const QList<QUrl> urls)
{
    Q_D(InfoCache);
    if (d->cacheWorkerStoped || urls.size() <= 0)
        return;

    // 读取主缓存，插入到
    d->status = kCacheCopy;
    QMap<QUrl, FileInfoPointer> infos;
    {
        QWriteLocker wlk(&d->mianLock);
        for (const auto &url : urls) {
            auto info = d->mainCache.take(url);
            if (info)
                infos.insert(url, d->mainCache.take(url));
        }
    }
    if (d->cacheWorkerStoped)
        return;
    // 断开监视器监视
    if (infos.size() > 0)
        emit cacheDisconnectWatcher(infos);
    // 移除时间队列
    emit cacheRemoveInfosTime(urls);

    // 设置读取主缓存和插入到主缓存中
    d->status = kCacheMain;
    {
        QWriteLocker wlk(&d->copyLock);
        for (const auto &url : urls) {
            d->copyCache.remove(url);
        }
    }
}
/*!
 * \brief getCacheInfo 获取文件
 *
 * \param QString 文件的url
 *
 * \return 文件的info
 */
FileInfoPointer InfoCache::getCacheInfo(const QUrl &url)
{
    Q_D(InfoCache);
    // 要异步线程重置计时器 todo 开辟一个可控制的线程来处理时间排序的问题
    // 读取副缓存和临时缓存返回
    FileInfoPointer info(nullptr);
    if (d->status == kCacheMain) {   // 可以读取主缓存返回
        QReadLocker wlk(&d->mianLock);
        info = d->mainCache.value(url);
    } else {
        QReadLocker wlk(&d->copyLock);
        info = d->copyCache.value(url);
    }
    // 异步线程或者信号更新时间
    // 使用线程处理加入时间序列问题
    if (info)
        emit cacheUpdateInfoTime(url);

    return info;
}
/*!
 * \brief refreshFileInfo 刷新缓存fileinfo
 *
 * \param QUrl 文件的URL
 *
 * \return
 */
void InfoCache::refreshFileInfo(const QUrl &url)
{
    FileInfoPointer info = getCacheInfo(url);
    if (info)
        info->refresh();
}
/*!
 * \brief timeRemoveCache 定时检查哪些fileinfo要移除
 *
 * \return
 */
void InfoCache::timeRemoveCache()
{
    Q_D(InfoCache);
    // 取出哪些url的时间超出了u
    qint64 delCount = d->urlTimeSortMap.size() < kCacheFileinfoCount ? 0 : d->urlTimeSortMap.size() - kCacheFileinfoCount;
    QList<QUrl> delList;
    foreach (const auto time, d->timeToUrlMap.uniqueKeys()) {
        if (d->cacheWorkerStoped)
            return;

        if (time < QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch() - kCacheRemoveTime)) {
            delList.append(d->timeToUrlMap.values(time));
            continue;
        }

        if (delList.size() >= delCount) {
            break;
        }

        delList.append(d->timeToUrlMap.values(time));
    }
    // 发送异步消息 告诉移除线程创建移除线程移除，考虑是否是使用线程一直还是使用临时线程（使用临时线程）
    if (delList.size() > 0 && !d->cacheWorkerStoped)
        emit cacheRemoveCaches(delList);
}

void InfoCache::removeInfosTimeWorker(const QList<QUrl> urls)
{
    for (auto url : urls) {
        if (d->urlTimeSortMap.contains(url)) {
            d->timeToUrlMap.remove(d->urlTimeSortMap.take(url));
        }
    }
}

void InfoCache::fileAttributeChanged(const QUrl url)
{
    refreshFileInfo(url);
}

CacheWorker::CacheWorker(QObject *parent)
    : QObject(parent)
{
}

CacheWorker::~CacheWorker()
{
}

void CacheWorker::cacheInfo(const QUrl url, const FileInfoPointer info)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    InfoCache::instance().cacheInfo(url, info);
}

void CacheWorker::removeCaches(const QList<QUrl> urls)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    InfoCache::instance().removeCaches(urls);
}

void CacheWorker::updateInfoTime(const QUrl url)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    InfoCache::instance().updateSortTimeWorker(url);
}

void CacheWorker::dealRemoveInfo()
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    InfoCache::instance().timeRemoveCache();
}

void CacheWorker::removeInfosTime(const QList<QUrl> urls)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    InfoCache::instance().removeInfosTimeWorker(urls);
}

void CacheWorker::disconnectWatcher(const QMap<QUrl, FileInfoPointer> infos)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    InfoCache::instance().disconnectWatcher(infos);
}

InfoCacheController::~InfoCacheController()
{
    removeTimer->stop();
    thread->quit();
    InfoCache::instance().stop();
    thread->wait();
}

InfoCacheController &InfoCacheController::instance()
{
    static InfoCacheController cacheController;
    return cacheController;
}

bool InfoCacheController::cacheDisable(const QString &scheme)
{
    return InfoCache::instance().cacheDisable(scheme);
}

void InfoCacheController::setCacheDisbale(const QString &scheme, bool disable)
{
    return InfoCache::instance().setCacheDisbale(scheme, disable);
}

FileInfoPointer InfoCacheController::getCacheInfo(const QUrl &url)
{
    return InfoCache::instance().getCacheInfo(url);
}

InfoCacheController::InfoCacheController(QObject *parent)
    : QObject(parent), thread(new QThread), worker(new CacheWorker), removeTimer(new QTimer)
{
    init();
}

void InfoCacheController::init()
{
    removeTimer->moveToThread(qApp->thread());
    connect(removeTimer.data(), &QTimer::timeout, worker.data(), &CacheWorker::dealRemoveInfo, Qt::QueuedConnection);
    connect(this, &InfoCacheController::cacheFileInfo, worker.data(), &CacheWorker::cacheInfo, Qt::QueuedConnection);
    connect(this, &InfoCacheController::removeCacheFileInfo, worker.data(), &CacheWorker::removeCaches, Qt::QueuedConnection);
    connect(&InfoCache::instance(), &InfoCache::cacheRemoveCaches, worker.data(), &CacheWorker::removeCaches, Qt::QueuedConnection);
    connect(&InfoCache::instance(), &InfoCache::cacheRemoveInfosTime, worker.data(), &CacheWorker::removeInfosTime, Qt::QueuedConnection);
    connect(&InfoCache::instance(), &InfoCache::cacheDisconnectWatcher, worker.data(), &CacheWorker::disconnectWatcher, Qt::QueuedConnection);

    worker->moveToThread(thread.data());
    thread->start();
    removeTimer->setInterval(kRotationTrainingTime);
    removeTimer->start();
}

}

//开线程移除缓存和同步缓存操作
