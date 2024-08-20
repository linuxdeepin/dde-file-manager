// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileprovider.h"
#include "filefilter.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>

#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

FileProvider::FileProvider(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<QUrl>>();
    connect(ThumbnailFactory::instance(), &ThumbnailFactory::produceFinished, this, &FileProvider::fileThumbUpdated);
    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FileProvider::onFileInfoUpdated, Qt::QueuedConnection);
}

FileProvider::~FileProvider()
{
    if (traversalThread) {
        traversalThread->disconnect(this);
        traversalThread->stopAndDeleteLater();
    }
}

bool FileProvider::setRoot(const QUrl &url)
{
    if (!url.isValid()) {
        fmWarning() << "invaild url:" << url;
        return false;
    }

    fmInfo() << "set root url" << url;
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
        fmInfo() << "file watcher is started.";
        return true;
    }

    fmWarning() << "fail to create watcher for" << url;
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
    if (traversalThread) {
        traversalThread->disconnect(this);
        traversalThread->stopAndDeleteLater();
    }

    traversalThread = new TraversalDirThread(rootUrl, QStringList(), filters, QDirIterator::NoIteratorFlags);
    traversalThread->setQueryAttributes("standard::standard::name");
    connect(traversalThread, &TraversalDirThread::updateChildren, this, &FileProvider::reset);
    connect(traversalThread, &TraversalDirThread::finished, this, &FileProvider::traversalFinished);

    // get file property in thread of TraversalDir to improve performance.
    connect(traversalThread, &TraversalDirThread::updateChild, this, &FileProvider::preupdateData, Qt::DirectConnection);

    updateing = true;
    traversalThread->start();
    fmDebug() << "start file traversal";
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
            fmDebug() << "TraversalFilter returns true: it is invalid";
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
                              [&url](const QSharedPointer<FileFilter> &filter) {
                                  return filter->fileCreatedFilter(url);
                              });

    if (!ignore)
        emit fileInserted(url);
}

void FileProvider::remove(const QUrl &url)
{
    for (const auto &filter : fileFilters) {
        if (filter->fileDeletedFilter(url))
            fmWarning() << "DeletedFilter returns true: it is invalid";
    }

    emit fileRemoved(url);
}

void FileProvider::rename(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ignore = std::any_of(fileFilters.begin(), fileFilters.end(),
                              [&oldUrl, &newUrl](const QSharedPointer<FileFilter> &filter) {
                                  return filter->fileRenameFilter(oldUrl, newUrl);
                              });

    emit fileRenamed(oldUrl, ignore ? QUrl() : newUrl);
}

void FileProvider::update(const QUrl &url)
{
    if (UrlRoute::urlParent(url) != rootUrl)
        return;
    bool ignore = std::any_of(fileFilters.begin(), fileFilters.end(),
                              [&url](const QSharedPointer<FileFilter> &filter) {
                                  return filter->fileUpdatedFilter(url);
                              });

    if (!ignore)
        emit fileUpdated(url);
}

void FileProvider::preupdateData(const QUrl &url)
{
    if (!url.isValid())
        return;

    // check if there is cache
    auto cachedInfo = InfoCacheController::instance().getCacheInfo(url);

    // file info that is slow at first using should be called there to cache it.
    auto info = InfoFactory::create<FileInfo>(url);
    if (updateing && info) {

        // cached file need to refresh.
        if (info == cachedInfo)
            info->updateAttributes();

        // get file mime type for sorting.
        info->fileMimeType();
    }
}

void FileProvider::onFileInfoUpdated(const QUrl &url, const QString &infoPtr, const bool isLinkOrg)
{
    Q_UNUSED(infoPtr);
    if (UrlRoute::urlParent(url) != rootUrl)
        return;
    emit fileInfoUpdated(url, isLinkOrg);
}
