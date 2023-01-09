/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "recentfilewatcher.h"
#include "utils/recentmanager.h"
#include "private/recentfilewatcher_p.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"

DFMBASE_USE_NAMESPACE

namespace dfmplugin_recent {

RecentFileWatcherPrivate::RecentFileWatcherPrivate(const QUrl &fileUrl, RecentFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool RecentFileWatcherPrivate::start()
{
    return proxy && proxy->startWatcher();
}

bool RecentFileWatcherPrivate::stop()
{
    return proxy && proxy->stopWatcher();
}

void RecentFileWatcherPrivate::initFileWatcher()
{
    QUrl watchUrl = QUrl::fromLocalFile(path);

    proxy = WatcherFactory::create<AbstractFileWatcher>(watchUrl);

    if (!proxy) {
        qWarning("watcher create failed.");
        abort();
    }
}

void RecentFileWatcherPrivate::initConnect()
{
    connect(proxy.data(), &AbstractFileWatcher::fileDeleted, q, &AbstractFileWatcher::fileDeleted);
    connect(proxy.data(), &AbstractFileWatcher::fileAttributeChanged, q, &AbstractFileWatcher::fileAttributeChanged);
    connect(proxy.data(), &AbstractFileWatcher::subfileCreated, q, &AbstractFileWatcher::subfileCreated);

    auto onParentDeleted = [=](const QString &, const QString &deletedPath) {
        if (path.startsWith(deletedPath) && !deletedPath.isEmpty()) {
            qDebug() << "recent: watched: " << path << ", deleted: " << deletedPath;
            Q_EMIT q->fileDeleted(QUrl::fromLocalFile(path));
        }
    };
    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, onParentDeleted);
    connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved, this, onParentDeleted);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, onParentDeleted);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevRemoved, this, onParentDeleted);
}

RecentFileWatcher::RecentFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new RecentFileWatcherPrivate(url, this), parent)
{
    dptr = static_cast<RecentFileWatcherPrivate *>(d.data());
    dptr->initFileWatcher();
    dptr->initConnect();
}

RecentFileWatcher::~RecentFileWatcher()
{
}

void RecentFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
    if (subfileUrl.scheme() != RecentManager::scheme())
        return;
    if (enabled) {
        addWatcher(subfileUrl);
    } else {
        removeWatcher(subfileUrl);
    }
}

void RecentFileWatcher::addWatcher(const QUrl &url)
{
    if (!url.isValid() || dptr->urlToWatcherMap.contains(url)) {
        return;
    }

    AbstractFileWatcherPointer watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (!watcher)
        return;

    watcher->moveToThread(this->thread());

    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentFileWatcher::onFileAttributeChanged);
    connect(watcher.data(), &AbstractFileWatcher::fileDeleted, this, &RecentFileWatcher::onFileDeleted);

    dptr->urlToWatcherMap[url] = watcher;

    if (dptr->started) {
        watcher->startWatcher();
    }
}

void RecentFileWatcher::removeWatcher(const QUrl &url)
{
    AbstractFileWatcherPointer watcher = dptr->urlToWatcherMap.take(url);

    if (!watcher) {
        return;
    }
}

void RecentFileWatcher::onFileDeleted(const QUrl &url)
{
    QUrl newUrl = QUrl::fromLocalFile(url.path());
    newUrl.setScheme(RecentManager::scheme());
    removeWatcher(newUrl);
    RecentManager::instance()->removeRecentFile(newUrl);

    emit fileDeleted(newUrl);
}

void RecentFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    QUrl newUrl = url;
    newUrl.setScheme(RecentManager::scheme());

    emit fileAttributeChanged(newUrl);
}

}
