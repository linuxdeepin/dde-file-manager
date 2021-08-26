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

#include "dfminfocachesmanager.h"
#include "base/private/dfminfocachesmanager_p.h"
#include "base/dfmschemefactory.h"

#include <QSharedPointer>
//更新间隔时间
#define REFRESH_SPACE_TIME 500
//文件缓存文件的总个数
#define CACHE_FILEINFO_COUNT 100000
//文件缓存移除的时间
#define CACHE_REMOVE_TIME 5 * 60 * 1000

Q_GLOBAL_STATIC(DFMInfoCachesManager,_InfoCacheManager)

ReFreshThread::ReFreshThread(QObject *parent)
    : QThread(parent)
{
    time.start();
}

void ReFreshThread::refreshFileInfoByUrl(const QUrl &url)
{
    DAbstractFileInfoPointer fileinfo = DFMInfoCachesManager::instance().getCacheInfo(url);
    if (!fileinfo) {
        if (m_needRefreshMap.contains(url))
            m_removedCacheList.push_back(url);
        m_refreshMap.remove(url);
        return;
    }
    qint64 reflreshTime = m_refreshMap.value(url);
    if (time.elapsed() - reflreshTime > 500) {
        updateRefreshTimeByUrl(url);
        fileinfo->refresh();
    }
    else {
        m_needRefreshMap.insert(url, reflreshTime);
    }
}

void ReFreshThread::updateRefreshTimeByUrl(const QUrl &url)
{
    m_refreshMap.insert(url,time.elapsed());
}

void ReFreshThread::removeRefreshByUrl(const QUrl &url)
{
    m_refreshMap.remove(url);
    if (m_needRefreshMap.contains(url)) {
        m_removedCacheList.push_back(url);
    }
}

void ReFreshThread::stopRefresh()
{
    m_bStop = true;
    wait();
}

void ReFreshThread::run()
{
    if (m_bStop)
        return;
    QMap<QUrl, qint64>::iterator itr = m_needRefreshMap.begin();
    QMap<QUrl, qint64>::iterator itrEnd = m_needRefreshMap.end();
    while(itr != itrEnd) {
        DAbstractFileInfoPointer fileinfo = DFMInfoCachesManager::instance().getCacheInfo(itr.key());
        if (!fileinfo)
            continue;
        if (m_bStop)
            return;
        if (m_removedCacheList.contains(itr.key())) {
            m_removedCacheList.removeAll(itr.key());
            itr = m_needRefreshMap.erase(itr);
            continue;
        }

        if (time.elapsed() - itr.value() >= REFRESH_SPACE_TIME) {
            updateRefreshTimeByUrl(itr.key());
            fileinfo->refresh();
            itr = m_needRefreshMap.erase(itr);
            itrEnd = m_needRefreshMap.end();
        } else
            ++itr;
    }
    QThread::msleep(REFRESH_SPACE_TIME);
}

DFMInfoCachesManagerPrivate::DFMInfoCachesManagerPrivate(DFMInfoCachesManager *qq)
    : q_ptr(qq)
{
    m_refreshThread.reset(new ReFreshThread);
    m_refreshThread->start();
}

DFMInfoCachesManagerPrivate::~DFMInfoCachesManagerPrivate()
{
    if (m_refreshThread) {
        m_refreshThread->stopRefresh();
        m_refreshThread->deleteLater();
    }
}

void DFMInfoCachesManagerPrivate::updateSortByTimeCacheUrlList(const QUrl &url)
{
    if (m_sortByTimeCacheUrl.contains(url))
        m_sortByTimeCacheUrl.removeAll(url);
    m_sortByTimeCacheUrl.push_back(url);
}

DFMInfoCachesManager::DFMInfoCachesManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMInfoCachesManagerPrivate(this))
{
    connect(&d_ptr->m_removeTimer, &QTimer::timeout, this, &DFMInfoCachesManager::timeRemoveCache);
    d_ptr->m_removeTimer.setInterval(CACHE_REMOVE_TIME);
    d_ptr->m_removeTimer.start();
    connect(&d_ptr->m_needRemoveTimer, &QTimer::timeout, this, &DFMInfoCachesManager::timeNeedRemoveCache);
    d_ptr->m_needRemoveTimer.start(1000);
    d_ptr->m_needRemoveTimer.setInterval(CACHE_REMOVE_TIME);
}

DFMInfoCachesManager::~DFMInfoCachesManager()
{
    Q_D(DFMInfoCachesManager);
    d->m_removeTimer.stop();
    d->m_removeTimer.deleteLater();
}

DFMInfoCachesManager &DFMInfoCachesManager::instance()
{
    return *_InfoCacheManager;
}

DAbstractFileInfoPointer DFMInfoCachesManager::getCacheInfo(const QUrl &url)
{
    Q_D(DFMInfoCachesManager);
    if (d->m_fileInfos.contains(url)) {
        d->updateSortByTimeCacheUrlList(url);
        if (d->m_needRemoveCacheList.contains(url))
        {
            d->m_removedCacheList.push_back(url);
        }
    }
    return d->m_fileInfos.value(url);
}

void DFMInfoCachesManager::cacheInfo(const QUrl &url, const DAbstractFileInfoPointer &info)
{
    Q_D(DFMInfoCachesManager);

    //获取监视器，监听当前的file的改变
    QSharedPointer<DAbstractFileWatcher>  watcher = DFMWacherFactory::instance().create<DAbstractFileWatcher>(url);
    if (watcher) {
        connect(watcher.data(), &DAbstractFileWatcher::fileDeleted, this,  &DFMInfoCachesManager::refreshFileInfo);
        connect(watcher.data(), &DAbstractFileWatcher::fileModified, this,  &DFMInfoCachesManager::refreshFileInfo);
        connect(watcher.data(), &DAbstractFileWatcher::fileMoved, this,  &DFMInfoCachesManager::refreshFileInfo);
    }

    d->m_fileInfos.insert(url, info);
    d->updateSortByTimeCacheUrlList(url);
    d->m_refreshThread->updateRefreshTimeByUrl(url);
    //超过缓存总数量时，直接移除缓存，将移除的url加入到已移除队列
    if (d->m_sortByTimeCacheUrl.count() > CACHE_FILEINFO_COUNT) {
        QUrl removeUrl = d->m_sortByTimeCacheUrl.first() == nullptr ?
                    QUrl() : *(d->m_sortByTimeCacheUrl.first());
        removeCacheInfo(removeUrl);
        //已加入到待remove的链表，加入到m_removedCacheList链表
        if (d->m_needRemoveCacheList.contains(removeUrl))
            d->m_removedCacheList.push_back(removeUrl);
        //还没加入待remove的链表，加入到m_removedSortByTimeCacheList
        if (d->m_sortByTimeCacheUrl.contains(removeUrl))
            d->m_removedSortByTimeCacheList.push_back(removeUrl);
    }
}

void DFMInfoCachesManager::removeCacheInfo(const QUrl &url)
{
    Q_D(DFMInfoCachesManager);
    // 断开信号连接
    QSharedPointer<DAbstractFileWatcher>  watcher = DFMWacherFactory::instance().create<DAbstractFileWatcher>(url);
    if (watcher) {
        disconnect(watcher.data());
        DFMWatcherCachesManager::instance().removCacheWatcher(url);
    }
    d->m_fileInfos.remove(url);
    //移除刷新的url
    d->m_refreshThread->removeRefreshByUrl(url);
}

void DFMInfoCachesManager::refreshFileInfo(const QUrl &url)
{
    Q_D(DFMInfoCachesManager);

    if (!d->m_refreshThread)
        return;

    d->m_refreshThread->refreshFileInfoByUrl(url);

}

void DFMInfoCachesManager::timeNeedRemoveCache()
{
    Q_D(DFMInfoCachesManager);
    QList<QUrl>::iterator itr = d->m_sortByTimeCacheUrl.begin();
    QList<QUrl>::iterator itrEnd = d->m_sortByTimeCacheUrl.end();
    while(itr != itrEnd) {
        //移除的直接移除当前的url和移除移除的url
        if (d->m_removedSortByTimeCacheList.contains(*itr)) {
            itr = d->m_sortByTimeCacheUrl.erase(itr);
            d->m_removedSortByTimeCacheList.removeAll(*itr);
            continue;
        }
        //如果文件缓存中没有当前url的直接移除当前的url，和按时间排序的url
        DAbstractFileInfoPointer info = d->m_fileInfos.value((*itr));
        if (!info) {
            itr = d->m_sortByTimeCacheUrl.erase(itr);
            continue;
        }
        //插入待移除队列并移除时间排序的url
        if (info->ref == 1) {
            itr = d->m_sortByTimeCacheUrl.erase(itr);
            d->m_needRemoveCacheList.push_back(*itr);
            continue;
        }
        itr++;
    }
}

//将共享指针引用计数小于1的缓存文件信息的url加入到待析构队列
void DFMInfoCachesManager::timeRemoveCache()
{
    Q_D(DFMInfoCachesManager);
    QList<QUrl>::iterator itr = d->m_needRemoveCacheList.begin();
    QList<QUrl>::iterator itrEnd = d->m_needRemoveCacheList.end();
    while(itr != itrEnd) {
        //已被析构了的info，直接移除和从移除的列表中移除
        if (d->m_removedCacheList.contains(*itr)) {
            itr = d->m_needRemoveCacheList.erase(itr);
            d->m_removedCacheList.removeAll(*itr);
            continue;
        }

        // 移除文件信息缓存
        removeCacheInfo(*itr);

        // 移除当前需要移除缓存列表
        itr = d->m_needRemoveCacheList.erase(itr);
    }

}
