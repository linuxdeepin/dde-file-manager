// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharewatchermanager.h"

#include <dfm-base/file/local/localfilewatcher.h>

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
