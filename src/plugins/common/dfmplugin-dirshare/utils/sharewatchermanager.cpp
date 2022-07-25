/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "sharewatchermanager.h"

#include "dfm-base/file/local/localfilewatcher.h"

using namespace dfmplugin_dirshare;
DFMBASE_USE_NAMESPACE

ShareWatcherManager::ShareWatcherManager(QObject *parent)
    : QObject(parent)
{
}

ShareWatcherManager::~ShareWatcherManager()
{
}

LocalFileWatcher *ShareWatcherManager::add(const QString &path)
{
    auto watcher = watchers.value(path);
    if (watcher)
        return watcher;

    watcher = new LocalFileWatcher(QUrl::fromLocalFile(path), this);
    watchers.insert(path, watcher);

    connect(watcher, &LocalFileWatcher::fileAttributeChanged, this, [this](const QUrl &url) { Q_EMIT this->fileAttributeChanged(url.toLocalFile()); });
    connect(watcher, &LocalFileWatcher::fileDeleted, this, [this](const QUrl &url) { Q_EMIT this->fileDeleted(url.toLocalFile()); });
    connect(watcher, &LocalFileWatcher::subfileCreated, this, [this](const QUrl &url) { Q_EMIT this->subfileCreated(url.toLocalFile()); });
    connect(watcher, &LocalFileWatcher::fileRename, this, [this](const QUrl &oldUrl, const QUrl &newUrl) { Q_EMIT this->fileMoved(oldUrl.toLocalFile(), newUrl.toLocalFile()); });

    watcher->startWatcher();
    return watcher;
}

void ShareWatcherManager::remove(const QString &path)
{
    auto watcher = watchers.take(path);
    if (watcher)
        watcher->deleteLater();
}
