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
#include "dfilewatchermanager.h"
#include "dfilewatcher.h"
#include "durl.h"

DFM_BEGIN_NAMESPACE

class DFileWatcherManagerPrivate
{
public:
    DFileWatcherManagerPrivate();

    QMap<QString, DFileWatcher*> watchersMap;
};

DFileWatcherManagerPrivate::DFileWatcherManagerPrivate()
{

}

DFileWatcherManager::DFileWatcherManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFileWatcherManagerPrivate())
{

}

DFileWatcherManager::~DFileWatcherManager()
{

}

DFileWatcher *DFileWatcherManager::add(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.value(filePath);

    if (watcher)
        return watcher;

    watcher = new DFileWatcher(filePath, this);

    connect(watcher, &DFileWatcher::fileAttributeChanged, this, [this](const DUrl &url) {
        emit fileAttributeChanged(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileClosed, this, [this](const DUrl &url) {
        emit fileClosed(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileDeleted, this, [this](const DUrl &url) {
        emit fileDeleted(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileModified, this, [this](const DUrl &url) {
        emit fileModified(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileMoved, this, [this](const DUrl &fromUrl, const DUrl &toUrl) {
        emit fileMoved(fromUrl.toLocalFile(), toUrl.toLocalFile());
    });
    connect(watcher, &DFileWatcher::subfileCreated, this, [this](const DUrl &url) {
        emit subfileCreated(url.toLocalFile());
    });

    d->watchersMap[filePath] = watcher;
    watcher->startWatcher();

    return watcher;
}

void DFileWatcherManager::remove(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.take(filePath);

    if (watcher)
        watcher->deleteLater();
}

DFM_END_NAMESPACE
