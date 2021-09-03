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

#ifndef INFOCACHE_H
#define INFOCACHE_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/threadcontainer.hpp"
#include "dfm-base/base/abstractfileinfo.h"

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QUrl>

DFMBASE_BEGIN_NAMESPACE
class InfoCachePrivate;
// 异步刷新线程
class ReFreshThread : public QThread
{
    Q_OBJECT
public:
    explicit ReFreshThread(QObject *parent = nullptr);
    void refreshFileInfoByUrl(const QUrl &url);
    void updateRefreshTimeByUrl(const QUrl &url);
    void stopRefresh();
    void removeRefreshByUrl(const QUrl &url);
protected:
    void run() override;
private:
    DThreadMap<QUrl, qint64> needRefreshMap; // 待更新链表
    DThreadMap<QUrl, qint64> refreshMap; // 刷新时间
    DThreadList<QUrl> removedCacheList; // 已被removecache的url
    QTime time; // 计时器
    QAtomicInteger<bool> bStop = false; // 停止标志
};


class InfoFactory;
class InfoCachePrivate;
class InfoCache : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), InfoCache)
    QScopedPointer<InfoCachePrivate> d_ptr;
    friend InfoFactory;
    friend ReFreshThread;

public:
    explicit InfoCache(QObject *parent = nullptr);
    virtual ~InfoCache();
    static InfoCache &instance();
    AbstractFileInfoPointer getCacheInfo(const QUrl &url);
    void cacheInfo(const QUrl &url, const AbstractFileInfoPointer &info);
    void removeCacheInfo(const QUrl &url);
private Q_SLOTS:
    void refreshFileInfo(const QUrl &url, const DFMIO::DFileInfo &fileInfo);
    void timeNeedRemoveCache();
    void timeRemoveCache();
};
DFMBASE_END_NAMESPACE

#endif // INFOCACHE_H
