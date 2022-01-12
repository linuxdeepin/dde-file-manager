/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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

#include "recentdiriterator.h"
#include "recentutil.h"
#include "recentfileinfo.h"
#include "recentiterateworker.h"
#include "private/recentdiriterator_p.h"

#include "dfm-base/base/schemefactory.h"

DPRECENT_BEGIN_NAMESPACE

RecentDirIteratorPrivate::RecentDirIteratorPrivate(RecentDirIterator *qq)
    : QObject(nullptr),
      q(qq)
{

    RecentIterateWorker *worker = new RecentIterateWorker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &RecentDirIteratorPrivate::asyncHandleFileChanged, worker, &RecentIterateWorker::doWork);
    connect(worker, &RecentIterateWorker::recentUrls, this, &RecentDirIteratorPrivate::handleFileChanged);
    workerThread.start();

    emit asyncHandleFileChanged();
    watcher = WacherFactory::create<AbstractFileWatcher>(QUrl::fromLocalFile(RecentUtil::xbelPath()));
    connect(watcher.data(), &AbstractFileWatcher::subfileCreated, this, &RecentDirIteratorPrivate::asyncHandleFileChanged);
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentDirIteratorPrivate::asyncHandleFileChanged);
    watcher->startWatcher();
    for (QUrl url : recentNodes.keys())
        urlList << url;
}

RecentDirIteratorPrivate::~RecentDirIteratorPrivate()
{
}

void RecentDirIteratorPrivate::handleFileChanged(QList<QPair<QUrl, qint64>> &results)
{
    QList<QUrl> urlList;
    for (auto pair : results) {
        const QUrl &url = pair.first;
        urlList << url;
        if (!recentNodes.contains(url)) {
            recentNodes[url] = QSharedPointer<RecentFileInfo>(new RecentFileInfo(url));
            QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentUtil::rootUrl());
            if (watcher) {
                emit watcher->subfileCreated(url);
            }
        }
    }

    // delete does not exist url.
    for (auto iter = recentNodes.begin(); iter != recentNodes.end();) {

        const QUrl url = iter.key();
        if (!urlList.contains(url)) {
            iter = recentNodes.erase(iter);
            QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentUtil::rootUrl());
            if (watcher) {
                emit watcher->fileDeleted(url);
            }

        } else {
            auto info = iter.value();
            if (info) {
                // Todo(yanghao):updateInfo
                //               iter.value()->updateInfo();
                ++iter;
            } else {
                iter = recentNodes.erase(iter);
            }
        }
    }
}

RecentDirIterator::RecentDirIterator(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new RecentDirIteratorPrivate(this))
{
}

RecentDirIterator::~RecentDirIterator()
{
    if (d) {
        d->watcher->stopWatcher();
        d->workerThread.quit();
        d->workerThread.wait();
        delete d;
    }
}

QUrl RecentDirIterator::next()
{
    if (!d->urlList.isEmpty()) {
        d->currentUrl = d->urlList.dequeue();
        return d->currentUrl;
    }
    return QUrl();
}

bool RecentDirIterator::hasNext() const
{
    if (!d->urlList.isEmpty()) {
        return true;
    }

    return false;
}

QString RecentDirIterator::fileName() const
{
    AbstractFileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->fileName() : QString();
}

QUrl RecentDirIterator::fileUrl() const
{
    AbstractFileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->url() : QString();
}

const AbstractFileInfoPointer RecentDirIterator::fileInfo() const
{
    return d->recentNodes.value(d->currentUrl);
}

QUrl RecentDirIterator::url() const
{
    return RecentUtil::rootUrl();
}

DPRECENT_END_NAMESPACE
