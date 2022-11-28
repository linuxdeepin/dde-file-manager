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
#ifndef INFOCACHE_H
#define INFOCACHE_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/threadcontainer.hpp"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QTime>
#include <QUrl>

namespace dfmbase {
class InfoCachePrivate;
class InfoCache;

// 异步缓存和移除
class Worker : public QObject
{
    Q_OBJECT
    friend class InfoCacheController;

public:
    ~Worker();
Q_SIGNALS:
    void workerCacheInfo(const QUrl url, const AbstractFileInfoPointer info);
    void workerRemoveCaches(const QList<QUrl> urls);
    void workerUpdateInfoTime(const QUrl url);
    void workerDealRemoveInfo();
    void workerRemoveInfosTime(const QList<QUrl> urls);
    void workerDisconnectWatcher(const QMap<QUrl, AbstractFileInfoPointer> infos);
public Q_SLOTS:
    void cacheInfo(const QUrl url, const AbstractFileInfoPointer info);
    void removeCaches(const QList<QUrl> urls);
    void updateInfoTime(const QUrl url);
    void dealRemoveInfo();
    void removeInfosTime(const QList<QUrl> urls);
    void disconnectWatcher(const QMap<QUrl, AbstractFileInfoPointer> infos);

private:
    explicit Worker(QObject *parent = nullptr);
};

class InfoCachePrivate;
class InfoCache : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d), InfoCache)
    QScopedPointer<InfoCachePrivate> d;
    friend class InfoCacheController;

public:
    virtual ~InfoCache();

Q_SIGNALS:
    void cacheRemoveCaches(const QList<QUrl> &key);
    void cacheDisconnectWatcher(const QMap<QUrl, AbstractFileInfoPointer> infos);
    void cacheUpdateInfoTime(const QUrl url);
    void cacheRemoveInfosTime(const QList<QUrl> urls);

private:
    explicit InfoCache(QObject *parent = nullptr);
    bool cacheDisable(const QString &scheme);
    void setCacheDisbale(const QString &scheme, bool disable = true);
    AbstractFileInfoPointer getCacheInfo(const QUrl &url);
    void stop();

private Q_SLOTS:
    void fileAttributeChanged(const QUrl url);
    void removeCache(const QUrl url);
    void cacheInfo(const QUrl url, const AbstractFileInfoPointer info);
    void disconnectWatcher(const QMap<QUrl, AbstractFileInfoPointer> infos);
    void removeCaches(const QList<QUrl> urls);
    void updateSortTimeWorker(const QUrl url);
    void timeRemoveCache();
    void removeInfosTimeWorker(const QList<QUrl> urls);
    void refreshFileInfo(const QUrl &url);
};

class InfoCacheController : public QObject
{
    Q_OBJECT
    QSharedPointer<QThread> thread { nullptr };
    QSharedPointer<Worker> worker { nullptr };
    QSharedPointer<InfoCache> cache { nullptr };
    QSharedPointer<QTimer> removeTimer { nullptr };   // 移除缓存的
public:
    virtual ~InfoCacheController();
    static InfoCacheController &instance();
    bool cacheDisable(const QString &scheme);
    void setCacheDisbale(const QString &scheme, bool disable = true);
    AbstractFileInfoPointer getCacheInfo(const QUrl &url);
Q_SIGNALS:
    void cacheFileInfo(const QUrl url, const AbstractFileInfoPointer info);

private:
    explicit InfoCacheController(QObject *parent = nullptr);
    void init();

private:
    static InfoCacheController *cacheController;
};
}

#endif   // INFOCACHE_H
