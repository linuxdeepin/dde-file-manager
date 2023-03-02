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

    // get file property in thread of TraversalDir to improve performance.
    connect(traversalThread.get(), &TraversalDirThread::updateChild, this, &FileProvider::preupdateData, Qt::DirectConnection);

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
    QList<QUrl> urls { children };
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
    bool ignore = std::any_of(fileFilters.begin(), fileFilters.end(),
                                  [&url](const QSharedPointer<FileFilter> &filter){
            return filter->fileCreatedFilter(url);
        });

    if (!ignore)
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
    bool ignore = std::any_of(fileFilters.begin(), fileFilters.end(),
                              [&oldUrl, &newUrl](const QSharedPointer<FileFilter> &filter){
        return filter->fileRenameFilter(oldUrl, newUrl);
    });

    emit fileRenamed(oldUrl, ignore ? QUrl() : newUrl);
}

void FileProvider::update(const QUrl &url)
{
    if (UrlRoute::urlParent(url) != rootUrl && url != rootUrl)
        return;
    bool ignore = std::any_of(fileFilters.begin(), fileFilters.end(),
                                  [&url](const QSharedPointer<FileFilter> &filter){
            return filter->fileUpdatedFilter(url);
        });

    if (!ignore)
        emit fileUpdated(url);
}

void FileProvider::preupdateData(const QUrl &url)
{
    if (!url.isValid())
        return;
    // file info that is slow at first using should be called there to cache it.
    auto info = InfoFactory::create<AbstractFileInfo>(url);
    if (updateing && info) {
        // get file mime type for sorting.
        info->fileMimeType();
    }
}
