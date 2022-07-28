/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "fileprovider.h"
#include "filefilter.h"

#include <base/schemefactory.h>

#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

FileProvider::FileProvider(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<QUrl>>();
}

bool FileProvider::setRoot(const QUrl &url)
{
    if (!url.isValid()) {
        qWarning() << "invaild url:" << url;
        return false;
    }

    rootUrl = url;
    if (watcher)
        watcher->disconnect(this);

    watcher = WatcherFactory::create<AbstractFileWatcher>(rootUrl);

    if (Q_LIKELY(!watcher.isNull())) {
        // using Qt::QueuedConnection to reduce UI jamming.
        connect(watcher.data(), &AbstractFileWatcher::fileDeleted, this, &FileProvider::remove, Qt::QueuedConnection);
        connect(watcher.data(), &AbstractFileWatcher::subfileCreated, this, &FileProvider::insert, Qt::QueuedConnection);
        connect(watcher.data(), &AbstractFileWatcher::fileRename, this, &FileProvider::rename, Qt::QueuedConnection);
        connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &FileProvider::update, Qt::QueuedConnection);
        watcher->startWatcher();
        qInfo() << "set root url" << url;
        return true;
    }

    qWarning() << "fail to create watcher for" << url;
    return false;
}

QUrl ddplugin_canvas::FileProvider::root() const
{
    return rootUrl;
}

bool FileProvider::isUpdating() const
{
    return updateing;
}

void FileProvider::refresh(QDir::Filters filters)
{
    updateing = false;
    if (!traversalThread.isNull()) {
        traversalThread->disconnect(this);
        traversalThread->stopAndDeleteLater();
    }

    traversalThread.reset(new TraversalDirThread(rootUrl, QStringList(), filters, QDirIterator::NoIteratorFlags));
    if (Q_UNLIKELY(traversalThread.isNull()))
        return;

    connect(traversalThread.get(), &TraversalDirThread::updateChildren, this, &FileProvider::reset);
    connect(traversalThread.get(), &TraversalDirThread::finished, this, &FileProvider::traversalFinished);
    updateing = true;
    traversalThread->start();
}

void FileProvider::installFileFilter(QSharedPointer<FileFilter> filter)
{
    if (fileFilters.contains(filter))
        return;

    fileFilters.append(filter);
}

void FileProvider::removeFileFilter(QSharedPointer<FileFilter> filter)
{
    fileFilters.removeOne(filter);
}

void FileProvider::reset(QList<QUrl> children)
{
    // desktop needs some file to show.
    // it has no file to show if don't emit refreshEnd when traversalFilter returns true .
    for (const auto &filter : fileFilters) {
        if (filter->fileTraversalFilter(children))
            qWarning() << "TraversalFilter returns true: it is invalid";
    }

    emit refreshEnd(children);
}

void FileProvider::traversalFinished()
{
    updateing = false;
    QApplication::restoreOverrideCursor();
}

void FileProvider::insert(const QUrl &url)
{
    for (const auto &filter : fileFilters)
        if (filter->fileCreatedFilter(url))
            return;

    emit fileInserted(url);
}

void FileProvider::remove(const QUrl &url)
{
    for (const auto &filter : fileFilters) {
        if (filter->fileDeletedFilter(url))
            qWarning() << "DeletedFilter returns true: it is invalid";
    }

    emit fileRemoved(url);
}

void FileProvider::rename(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ignore = false;
    for (const auto &filter : fileFilters) {
        if (filter->fileRenameFilter(oldUrl, newUrl)) {
            ignore = true;
            break;
        }
    }

    emit fileRenamed(oldUrl, ignore ? QUrl() : newUrl);
}

void FileProvider::update(const QUrl &url)
{
    for (const auto &filter : fileFilters)
        if (filter->fileUpdatedFilter(url))
            return;

    emit fileUpdated(url);
}
