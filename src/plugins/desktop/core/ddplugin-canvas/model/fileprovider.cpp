// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileprovider.h"
#include "filefilter.h"

#include <base/schemefactory.h>
#include <utils/fileinfohelper.h>

#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

FileProvider::FileProvider(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<QUrl>>();
    connect(&FileInfoHelper::instance(), &FileInfoHelper::createThumbnailFinished, this, &FileProvider::update);
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

void FileProvider::reset(QList<AbstractFileInfoPointer> children)
{
    // desktop needs some file to show.
    // it has no file to show if don't emit refreshEnd when traversalFilter returns true .
    QList<QUrl> urls;
    for (const auto &info : children) {
        if (!info)
            continue;
        urls.append(info->urlOf(UrlInfoType::kUrl));
    }
    for (const auto &filter : fileFilters) {
        if (filter->fileTraversalFilter(urls))
            qWarning() << "TraversalFilter returns true: it is invalid";
    }

    emit refreshEnd(urls);
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
    if (UrlRoute::urlParent(url) != rootUrl && url != rootUrl)
        return;

    for (const auto &filter : fileFilters)
        if (filter->fileUpdatedFilter(url))
            return;

    emit fileUpdated(url);
}
