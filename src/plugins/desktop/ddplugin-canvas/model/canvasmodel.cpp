/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#include "canvasmodel.h"
#include "canvasmodel_p.h"
#include "filetreater.h"
#include "view/operator/fileoperaterproxy.h"

#include <interfaces/abstractfileinfo.h>
#include <base/standardpaths.h>
#include <base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <base/application/application.h>
#include <base/application/settings.h>

#include <QDateTime>
#include <QMimeData>
#include <QTimer>

DFMBASE_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE

CanvasModelPrivate::CanvasModelPrivate(CanvasModel *qq)
    : QObject(qq), q(qq)
{
    fileTreater.reset(new FileTreater(qq));
}

CanvasModelPrivate::~CanvasModelPrivate()
{
}

void CanvasModelPrivate::onFileDeleted(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        QPair<EventType, QVariant> rmFile(kRmFile, QVariant(url));
        QVariant data = QVariant::fromValue(rmFile);
        watcherEvent.enqueue(data);
    }

    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void CanvasModelPrivate::onFileCreated(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        QPair<EventType, QVariant> rmFile(kAddFile, QVariant(url));
        QVariant data = QVariant::fromValue(rmFile);
        watcherEvent.enqueue(data);
    }

    metaObject()->invokeMethod(this, "doWatcherEvent", Qt::QueuedConnection);
}

void CanvasModelPrivate::onFileRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        QPair<QUrl, QUrl> urls(oldUrl, newUrl);
        QVariant varUrls = QVariant::fromValue(urls);
        QPair<EventType, QVariant> reFile(kReFile, varUrls);
        QVariant data = QVariant::fromValue(reFile);
        watcherEvent.enqueue(data);
    }

    metaObject()->invokeMethod(this, "doWatcherEvent", Qt::QueuedConnection);
}

void CanvasModelPrivate::onFileUpdated(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        QPair<EventType, QVariant> updateFile(kUpdateFile, QVariant(url));
        QVariant data = QVariant::fromValue(updateFile);
        watcherEvent.enqueue(data);
    }

    metaObject()->invokeMethod(this, "doWatcherEvent", Qt::QueuedConnection);
}

void CanvasModelPrivate::doWatcherEvent()
{
    if (processFileEventRuning)
        return;

    processFileEventRuning = true;
    while (checkFileEventQueue()) {
        QVariant event;
        {
            QMutexLocker lk(&watcherEventMutex);
            event = watcherEvent.dequeue();
        }

        if (!event.canConvert<QPair<EventType, QVariant>>()) {
            qInfo() << "data error format:" << event;
            continue;
        }

        QPair<EventType, QVariant> eventData = event.value<QPair<EventType, QVariant>>();
        const EventType eventType = eventData.first;

        if (kAddFile == eventType) {
            const QUrl &url = eventData.second.toUrl();
            fileTreater->insertChild(url);
        } else if (kRmFile == eventType) {
            const QUrl &url = eventData.second.toUrl();
            fileTreater->removeChild(url);
        } else if (kReFile == eventType) {
            const QPair<QUrl, QUrl> urls = eventData.second.value<QPair<QUrl, QUrl>>();
            const QUrl &oldUrl = urls.first;
            const QUrl &newUrl = urls.second;
            fileTreater->renameChild(oldUrl, newUrl);
        } else if (kUpdateFile == eventType) {
            const QUrl &url = eventData.second.toUrl();
            fileTreater->updateChild(url);
        }
    }
    processFileEventRuning = false;
}

void CanvasModelPrivate::delayRefresh(int ms)
{
    if (nullptr != refreshTimer.get())
        refreshTimer->stop();

    if (ms < 1) {
        doRefresh();
    } else {
        refreshTimer.reset(new QTimer);
        refreshTimer->setSingleShot(true);
        connect(refreshTimer.get(), &QTimer::timeout, this, &CanvasModelPrivate::doRefresh);
        refreshTimer->start(ms);
    }
}

void CanvasModelPrivate::onTraversalFinished()
{
    isUpdatedChildren = true;
    fileTreater->onTraversalFinished();
}

bool CanvasModelPrivate::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    bool isEmptyQueue = watcherEvent.isEmpty();
    return !isEmptyQueue;
}

void CanvasModelPrivate::doRefresh()
{
    isUpdatedChildren = false;
    if (!traversalThread.isNull()) {
        traversalThread->disconnect();
        traversalThread->stopAndDeleteLater();
    }

    traversalThread.reset(new TraversalDirThread(rootUrl, QStringList(), filters, dfmio::DEnumerator::IteratorFlag::NoIteratorFlags));
    if (Q_UNLIKELY(traversalThread.isNull())) {
        isUpdatedChildren = true;
        return;
    }

    connect(traversalThread.data(), &TraversalDirThread::updateChildren, fileTreater.data(), &FileTreater::onUpdateChildren);
    connect(traversalThread.data(), &TraversalDirThread::finished, this, &CanvasModelPrivate::onTraversalFinished);

    traversalThread->start();
}

CanvasModel::CanvasModel(QObject *parent)
    : QAbstractItemModel(parent), d(new CanvasModelPrivate(this))
{
}

QModelIndex CanvasModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || column < 0 || d->fileTreater->childrenCount() <= row)
        return QModelIndex();

    if (auto fileInfo = d->fileTreater->fileInfo(row))
        return createIndex(row, column, fileInfo.data());

    return QModelIndex();
}

QModelIndex CanvasModel::index(const QUrl &fileUrl, int column)
{
    if (fileUrl.isEmpty())
        return QModelIndex();

    auto fileInfo = d->fileTreater->fileInfo(fileUrl);
    return index(fileInfo, column);
}

QModelIndex CanvasModel::index(const DFMLocalFileInfoPointer &fileInfo, int column) const
{
    if (!fileInfo)
        return QModelIndex();

    int row = (0 < d->fileTreater->childrenCount()) ? d->fileTreater->indexOfChild(fileInfo) : 0;
    return createIndex(row, column, const_cast<LocalFileInfo *>(fileInfo.data()));
}

QModelIndexList CanvasModel::indexs() const
{
    QModelIndexList results;
    for (int i = 0; i < rowCount(); i++) {
        QModelIndex childIndex = index(i);
        results << childIndex;
    }
    return results;
}

QModelIndex CanvasModel::parent(const QModelIndex &child) const
{
    if (child != rootIndex() && child.isValid())
        return rootIndex();

    return QModelIndex();
}

int CanvasModel::rowCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return d->fileTreater->childrenCount();

    return 0;
}

int CanvasModel::columnCount(const QModelIndex &parent) const
{
    if (parent == rootIndex())
        return 1;

    return 0;
}

QModelIndex CanvasModel::rootIndex() const
{
    return createIndex((quintptr)this, 0, (void *)this);
}

QVariant CanvasModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this || index == rootIndex())
        return QVariant();

    auto indexFileInfo = static_cast<LocalFileInfo *>(index.internalPointer());
    if (!indexFileInfo) {
        return QVariant();
    }
    switch (role) {
    case kFileIconRole:
        return indexFileInfo->fileIcon();
    case kFileNameRole:
        return indexFileInfo->fileName();
    case Qt::EditRole:
    case Qt::DisplayRole:
    case kFileDisplayNameRole:
        return indexFileInfo->fileDisplayName();
    case kFilePinyinName:
        return indexFileInfo->fileDisplayPinyinName();
    case kFileLastModifiedRole:
        return indexFileInfo->lastModified().toString();   // todo by file info: lastModifiedDisplayName
    case kFileSizeRole:
        return indexFileInfo->size();   // todo by file info: sizeDisplayName
    case kFileMimeTypeRole:
        return indexFileInfo->fileMimeType().name();   // todo by file info: mimeTypeDisplayName
    case kExtraProperties:
        return indexFileInfo->extraProperties();
    case kFileBaseNameRole:
        return indexFileInfo->baseName();
    case kFileSuffixRole:
        return indexFileInfo->suffix();
    case kFileNameOfRenameRole:
        return indexFileInfo->fileNameOfRename();
    case kFileBaseNameOfRenameRole:
        return indexFileInfo->baseNameOfRename();
    case kFileSuffixOfRenameRole:
        return indexFileInfo->suffixOfRename();
    default:
        return QString();
    }
}

Qt::ItemFlags CanvasModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    flags |= Qt::ItemIsDragEnabled;

    if (auto file = fileInfo(index)) {
        if (file->canRename())
            flags |= Qt::ItemIsEditable;

        if (file->isWritable())
            flags |= Qt::ItemIsDropEnabled;
        else
            flags |= Qt::ItemNeverHasChildren;

        // todo
        //flags &= ~file->fileItemDisableFlags();
    }

    return flags;
}

void CanvasModel::refresh(const QModelIndex &parent)
{
    if (parent != rootIndex())
        return;

    d->delayRefresh();
}

bool CanvasModel::isRefreshed() const
{
    return d->isUpdatedChildren;
}

QModelIndex CanvasModel::setRootUrl(QUrl url)
{
    if (url.isEmpty())
        url = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath));
    if (Q_UNLIKELY(!url.isValid())) {
        qWarning() << "root url is invalid:" << url;
        return rootIndex();
    }

    d->rootUrl = url;
    if (!d->watcher.isNull()) {
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileDeleted, d.data(), &CanvasModelPrivate::onFileDeleted);
        disconnect(d->watcher.data(), &AbstractFileWatcher::subfileCreated, d.data(), &CanvasModelPrivate::onFileCreated);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileRename, d.data(), &CanvasModelPrivate::onFileRename);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged, d.data(), &CanvasModelPrivate::onFileUpdated);
    }

    d->watcher = WacherFactory::create<AbstractFileWatcher>(d->rootUrl);
    if (Q_LIKELY(!d->watcher.isNull())) {
        connect(d->watcher.data(), &AbstractFileWatcher::fileDeleted, d.data(), &CanvasModelPrivate::onFileDeleted);
        connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated, d.data(), &CanvasModelPrivate::onFileCreated);
        connect(d->watcher.data(), &AbstractFileWatcher::fileRename, d.data(), &CanvasModelPrivate::onFileRename);
        connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged, d.data(), &CanvasModelPrivate::onFileUpdated);
        d->watcher->startWatcher();
    }

    d->filters = dfmio::DEnumerator::DirFilter::AllEntries | dfmio::DEnumerator::DirFilter::NoDotAndDotDot | dfmio::DEnumerator::DirFilter::System;
    d->whetherShowHiddenFile = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();
    if (d->whetherShowHiddenFile)
        d->filters |= dfmio::DEnumerator::DirFilter::Hidden;
    else
        d->filters &= ~static_cast<uint16_t>(dfmio::DEnumerator::DirFilter::Hidden);

    // root url changed,refresh data as soon as
    d->doRefresh();

    return rootIndex();
}

QUrl CanvasModel::rootUrl() const
{
    return d->rootUrl;
}

QUrl CanvasModel::url(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->rootUrl;

    if (index.isValid()) {
        if (auto info = d->fileTreater->fileInfo(index.row())) {
            return info->url();
        }
    }

    return QUrl();
}

DFMLocalFileInfoPointer CanvasModel::fileInfo(const QModelIndex &index) const
{
    if (index == rootIndex())
        return FileCreator->createFileInfo(d->rootUrl);

    if (!index.isValid())
        return nullptr;

    return d->fileTreater->fileInfo(index.row());
}

QList<QUrl> CanvasModel::files() const
{
    return d->fileTreater->files();
}

QMimeData *CanvasModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();
    QList<QUrl> urls;

    for (const QModelIndex &idx : indexes)
        urls << url(idx);

    data->setUrls(urls);
    return data;
}

bool CanvasModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    QList<QUrl> urlList = data->urls();
    if (urlList.isEmpty())
        return false;

    QUrl targetFileUrl;
    if (!parent.isValid() || parent == rootIndex()) {
        // drop file to desktop
        targetFileUrl = rootUrl();
        qInfo() << "drop file to desktop" << targetFileUrl << "data" << urlList << action;
    } else {
        targetFileUrl = url(parent);
        qInfo() << "drop file to " << targetFileUrl << "data:" << urlList << action;
    }

    auto itemInfo = FileCreator->createFileInfo(targetFileUrl);
    if (Q_UNLIKELY(!itemInfo))
        return false;

    if (itemInfo->isSymLink()) {
        targetFileUrl = itemInfo->symLinkTarget();
    }

    // todo Compress

    if (dfmbase::FileUtils::isTrashDesktopFile(targetFileUrl)) {
        FileOperaterProxyIns->dropToTrash(urlList);
        return true;
    } else if (dfmbase::FileUtils::isComputerDesktopFile(targetFileUrl)) {
        return true;
    } else if (dfmbase::FileUtils::isDesktopFile(targetFileUrl)) {
        FileOperaterProxyIns->dropToApp(urlList, targetFileUrl.toLocalFile());
        return true;
    }

    switch (action) {
    case Qt::CopyAction:
    case Qt::MoveAction:
        FileOperaterProxyIns->dropFiles(action, targetFileUrl, urlList);
        break;
    case Qt::LinkAction:
        break;
    default:
        return false;
    }

    return true;
}

QStringList CanvasModel::mimeTypes() const
{
    static QStringList types { QLatin1String("text/uri-list") };
    return types;
}

Qt::DropActions CanvasModel::supportedDragActions() const
{
    // todo
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions CanvasModel::supportedDropActions() const
{
    // todo
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

void CanvasModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)

    sort();
}

bool CanvasModel::sort()
{
    return d->fileTreater->sort();
}

Qt::SortOrder CanvasModel::sortOrder() const
{
    return d->fileTreater->sortOrder();
}

void CanvasModel::setSortOrder(const Qt::SortOrder &order)
{
    d->fileTreater->setSortOrder(order);
}

int CanvasModel::sortRole() const
{
    return d->fileTreater->sortRole();
}

void CanvasModel::setSortRole(dfmbase::AbstractFileInfo::SortKey role, Qt::SortOrder order)
{
    d->fileTreater->setSortRole(role, order);
}

bool CanvasModel::showHiddenFiles() const
{
    return d->whetherShowHiddenFile;
}

void CanvasModel::setShowHiddenFiles(const bool isShow)
{
    if (d->whetherShowHiddenFile == isShow)
        return;
    d->whetherShowHiddenFile = isShow;
    if (d->whetherShowHiddenFile)
        d->filters |= dfmio::DEnumerator::DirFilter::Hidden;
    else
        d->filters &= ~static_cast<uint16_t>(dfmio::DEnumerator::DirFilter::Hidden);
}

void CanvasModel::update()
{
    for (const QUrl &url : d->fileTreater->files())
        if (auto file = d->fileTreater->fileInfo(url))
            file->refresh();

    emit dataChanged(createIndex(0, 0), createIndex(rowCount(rootIndex()), 0));
}
