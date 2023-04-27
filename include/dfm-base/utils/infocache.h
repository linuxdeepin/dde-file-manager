// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INFOCACHE_H
#define INFOCACHE_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/threadcontainer.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QTime>
#include <QUrl>

namespace dfmbase {
class InfoCachePrivate;
class InfoCache;

// 异步缓存和移除
class CacheWorker : public QObject
{
    Q_OBJECT
    friend class InfoCacheController;

public:
    ~CacheWorker() override;
public Q_SLOTS:
    void cacheInfo(const QUrl url, const FileInfoPointer info);
    void removeCaches(const QList<QUrl> urls);
    void updateInfoTime(const QUrl url);
    void dealRemoveInfo();
    void removeInfosTime(const QList<QUrl> urls);
    void disconnectWatcher(const QMap<QUrl, FileInfoPointer> infos);

private:
    explicit CacheWorker(QObject *parent = nullptr);
};

class InfoCachePrivate;
class InfoCache : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InfoCache)
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d), InfoCache)
    QScopedPointer<InfoCachePrivate> d;
    friend class CacheWorker;
    friend class InfoCacheController;

public:
    virtual ~InfoCache() override;

Q_SIGNALS:
    void cacheRemoveCaches(const QList<QUrl> &key);
    void cacheDisconnectWatcher(const QMap<QUrl, FileInfoPointer> infos);
    void cacheUpdateInfoTime(const QUrl url);
    void cacheRemoveInfosTime(const QList<QUrl> urls);

private:
    explicit InfoCache(QObject *parent = nullptr);
    static InfoCache &instance();
    bool cacheDisable(const QString &scheme);
    void setCacheDisbale(const QString &scheme, bool disable = true);
    FileInfoPointer getCacheInfo(const QUrl &url);
    void stop();
    void cacheInfo(const QUrl url, const FileInfoPointer info);
    void disconnectWatcher(const QMap<QUrl, FileInfoPointer> infos);
    void removeCaches(const QList<QUrl> urls);
    void updateSortTimeWorker(const QUrl url);
    void timeRemoveCache();
    void removeInfosTimeWorker(const QList<QUrl> urls);

private Q_SLOTS:
    void fileAttributeChanged(const QUrl url);
    void removeCache(const QUrl url);
    void refreshFileInfo(const QUrl &url);
};

class InfoCacheController : public QObject
{
    Q_DISABLE_COPY(InfoCacheController)
    Q_OBJECT
    QSharedPointer<QThread> thread { nullptr };
    QSharedPointer<CacheWorker> worker { nullptr };
    QSharedPointer<QTimer> removeTimer { nullptr };   // 移除缓存的
public:
    virtual ~InfoCacheController() override;
    static InfoCacheController &instance();
    bool cacheDisable(const QString &scheme);
    void setCacheDisbale(const QString &scheme, bool disable = true);
    FileInfoPointer getCacheInfo(const QUrl &url);
Q_SIGNALS:
    void cacheFileInfo(const QUrl url, const FileInfoPointer info);
    void removeCacheFileInfo(const QList<QUrl> &urls);

private:
    explicit InfoCacheController(QObject *parent = nullptr);
    void init();
};
}   // namespace dfmbase

#endif   // INFOCACHE_H
