// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfilewatcher.h"
#include "utils/recentfilehelper.h"
#include "utils/recentmanager.h"
#include "private/recentfilewatcher_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>

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
        fmCritical() << "Watcher create failed for path:" << path;
        abort();
    }
}

void RecentFileWatcherPrivate::initConnect()
{
    connect(proxy.data(), &AbstractFileWatcher::fileDeleted, q, &AbstractFileWatcher::fileDeleted);
    connect(proxy.data(), &AbstractFileWatcher::fileAttributeChanged, q, &AbstractFileWatcher::fileAttributeChanged);
    connect(proxy.data(), &AbstractFileWatcher::subfileCreated, q, &AbstractFileWatcher::subfileCreated);
    connect(proxy.data(), &AbstractFileWatcher::fileRename, q, &AbstractFileWatcher::fileRename);

    auto onParentDeleted = [=](const QString &, const QString &deletedPath) {
        if (path.startsWith(deletedPath) && !deletedPath.isEmpty()) {
            fmInfo() << "Recent file parent directory deleted - watched:" << path << "deleted:" << deletedPath;
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
    if (subfileUrl.scheme() != RecentHelper::scheme())
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
    if (!watcher) {
        fmWarning() << "Failed to create watcher for URL:" << url;
        return;
    }

    watcher->moveToThread(qApp->thread());

    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentFileWatcher::onFileAttributeChanged);
    connect(watcher.data(), &AbstractFileWatcher::fileDeleted, this, &RecentFileWatcher::onFileDeleted);
    connect(watcher.data(), &AbstractFileWatcher::fileRename, this, &RecentFileWatcher::onFileRename);

    dptr->urlToWatcherMap[url] = watcher;

    if (dptr->started) {
        watcher->startWatcher();
    }
}

void RecentFileWatcher::removeWatcher(const QUrl &url)
{
    AbstractFileWatcherPointer watcher = dptr->urlToWatcherMap.take(url);

    if (!watcher) {
        fmDebug() << "No watcher found to remove for URL:" << url;
        return;
    }
}

QUrl RecentFileWatcher::getRealUrl(const QUrl &url)
{
    QUrl newUrl = QUrl::fromLocalFile(url.path());
    if (url.scheme() == Global::Scheme::kFtp || url.scheme() == Global::Scheme::kSmb) {
        for (const auto &watcher : dptr->urlToWatcherMap) {
            if (watcher.data() == sender()) {
                newUrl = watcher->url();
                return newUrl;
            }
        }
    }

    return newUrl;
}

void RecentFileWatcher::onFileDeleted(const QUrl &url)
{
    QUrl newUrl = getRealUrl(url);
    newUrl.setScheme(RecentHelper::scheme());
    removeWatcher(newUrl);
    RecentManager::instance()->removeRecentFile(newUrl);

    emit fileDeleted(newUrl);
}

void RecentFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    QUrl newUrl = getRealUrl(url);
    newUrl.setScheme(RecentHelper::scheme());
    emit fileAttributeChanged(newUrl);
}

void RecentFileWatcher::onFileRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(newUrl);
    QUrl newOldUrl = QUrl::fromLocalFile(oldUrl.path());
    newOldUrl.setScheme(RecentHelper::scheme());
    removeWatcher(newOldUrl);
    RecentManager::instance()->removeRecentFile(newOldUrl);

    emit fileDeleted(newOldUrl);
}
}
