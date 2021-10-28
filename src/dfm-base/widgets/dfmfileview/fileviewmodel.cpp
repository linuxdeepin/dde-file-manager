/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#include "fileviewmodel.h"
#include "private/fileviewmodel_p.h"
#include "fileview.h"

#include <QApplication>
#include <QPointer>

DFMBASE_BEGIN_NAMESPACE

FileViewModelPrivate::FileViewModelPrivate(FileViewModel *qq)
    : QObject (qq)
    , q(qq)
{

}

FileViewModelPrivate::~FileViewModelPrivate()
{
}
//文件的在当前目录下创建、修改、删除都需要顺序处理，并且都是在遍历目录完成后才有序的处理
//处理方式，受到这3个事件，都加入到事件处理队列
//判断当前遍历目录是否完成，是启动异步文件监视事件处理，否文件遍历完成时，启动异步文件监视事件处理
//在退出时清理异步事件处理，再次进行fetchmore时（遍历新目录时）清理文件监视事件。
void FileViewModelPrivate::doFileDeleted(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl,EventType>(url, RmFile));
    }
    if (isUpdatedChildren)
        return;
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);

}

void FileViewModelPrivate::dofileCreated(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl,EventType>(url, AddFile));
    }
    if (isUpdatedChildren)
        return;
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileViewModelPrivate::doFileUpdated(const QUrl &url)
{
    if (!updateurlList.contains(url))
        updateurlList.append(url);

    if (!updateTimer.isActive())
        updateTimer.start();
}

void FileViewModelPrivate::doFilesUpdated()
{
    if (updateurlList.count() <= 0) {
        updateTimer.stop();
        return;
    }

    QList<QUrl> fileUrls = updateurlList.list();
    updateurlList.clear();
    QPointer<FileViewModelPrivate> ptr = this;

    for (auto &fileUrl : fileUrls) {
        if (!childrenMap.contains(fileUrl))
            continue;
        int fileRow = childrens.indexOf(childrenMap.value(fileUrl));
        if (fileRow < 0)
            continue;

        q->updateViewItem(q->index(fileRow, 0));
    }
}

void FileViewModelPrivate::doUpdateChildren(const QList<FileViewItem*> &children)
{
    q->beginResetModel();
    childrens.setList(children);
    for (auto fileItem : children) {
        childrenMap.insert(fileItem->url(), fileItem);
    }
    q->endResetModel();
    isUpdatedChildren = true;
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void FileViewModelPrivate::doWatcherEvent()
{
    if (isUpdatedChildren)
        return;

    if (processFileEventRuning)
        return;

    processFileEventRuning = true;
    while (checkFileEventQueue()) {
        QPair<QUrl, EventType> event;
        {
            QMutexLocker lk(&watcherEventMutex);
            event = watcherEvent.dequeue();
        }
        const QUrl &fileUrl = event.first;

        if (fileUrl.isValid())
            continue;

        if (fileUrl == root->url()) {
            if (event.second == AddFile)
                continue;
            //todo:先不做
        }

        if (UrlRoute::urlParent(fileUrl) != root->url())
            continue;

        if (event.second == AddFile) {
            if (childrenMap.contains(fileUrl))
                continue;

            q->beginInsertRows(QModelIndex(), childrens.count(), childrens.count());
            FileViewItem *item = new FileViewItem(fileUrl);
            childrens.append(item);
            childrenMap.insert(fileUrl, item);
            q->endInsertRows();
        } else {
            if (!childrenMap.contains(fileUrl))
                continue;

            int fileIndex = childrens.indexOf(childrenMap.value(fileUrl));
            if (fileIndex == -1)
                continue;

            q->beginRemoveRows(QModelIndex(),fileIndex,fileIndex);
            childrens.removeOne(childrenMap.value(fileUrl));
            childrenMap.remove(fileUrl);
            q->endMoveRows();
        }

    }
    processFileEventRuning = false;
}

bool FileViewModelPrivate::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    bool isEmptyQueue = watcherEvent.isEmpty();
    return !isEmptyQueue;
}

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel (parent)
    , d(new FileViewModelPrivate(this))
{

}

FileViewModel::~FileViewModel()
{
    clear();
}

QModelIndex FileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(row < 0 || column < 0 || d->childrens.size() <= row)
        return  QModelIndex();

    return createIndex(row, column, d->childrens.at(row));
}

const FileViewItem *FileViewModel::itemFromIndex(const QModelIndex &index) const
{
    if (0 > index.row() || index.row() >= d->childrens.size())
        return nullptr;
    return d->childrens.at(index.row());
}

QModelIndex FileViewModel::setRootUrl(const QUrl &url)
{
    if (!d->root.isNull() && d->root->url() == url) {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        return createIndex(-1, 0, &d->root);
    }

    d->root.reset(new FileViewItem(url));
    QModelIndex root = createIndex(-1, 0, &d->root);

    if(!url.isValid())
        return root;

    d->childrens.clear();

    if (d->column == 0)
        d->column = 4;

    if (!d->watcher.isNull()) {
        disconnect(d->watcher.data());
    }
    d->watcher = WacherFactory::create<AbstractFileWatcher>(url);
    if (!d->watcher.isNull()) {
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::fileDeleted,
                         d.data(), &FileViewModelPrivate::doFileDeleted);
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated,
                         d.data(), &FileViewModelPrivate::dofileCreated);
        QObject::connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                         d.data(), &FileViewModelPrivate::doFileUpdated);
        d->watcher->startWatcher();
    }

    d->canFetchMoreFlag = true;
    return root;
}

QUrl FileViewModel::rootUrl()
{
    return d->root->fileinfo()->url();
}

AbstractFileInfoPointer FileViewModel::fileInfo(const QModelIndex &index)
{
    if (!index.isValid())
        return nullptr;
    if(index.row() < 0  || d->childrens.size() <= index.row())
        return  nullptr;
    return d->childrens.at(index.row())->fileinfo();
}

QModelIndex FileViewModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int FileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->childrens.size();
}

int FileViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->column;
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    auto item = itemFromIndex(index);
    if (item)
        return item->data(role);
    return QVariant();
}

void FileViewModel::clear()
{
    for (int x = 0; x < columnCount(); x++) {
        for (int y = 0; x < rowCount(); y++){
            delete itemFromIndex(index(x,y));
        }
    }
}
/*!
 * \brief FileViewModel::rowCountMaxShow
 * \return int 当前view最多展示多少行的item项
 */
int FileViewModel::rowCountMaxShow()
{
    // 优化思路，初始化计算行数，model设置相关行数，随后向下拉view时进行动态insert和界面刷新。
    // 避免一次性载入多个文件的长时间等待。
    auto view = qobject_cast<QAbstractItemView*>(QObject::parent());
    auto beginIndex = view->indexAt(QPoint{1,1});
    auto currViewHeight = view->size().height();
    auto currIndexHeight = beginIndex.data(Qt::SizeHintRole).toSize().height();
    if (currIndexHeight <= 0)
        return -1;
    return (currViewHeight / currIndexHeight) + 1;
}

void FileViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    d->isUpdatedChildren = false;
    if (!d->traversalThread.isNull()) {
        d->traversalThread->quit();
        d->traversalThread->wait();
        disconnect(d->traversalThread.data());
    }
    d->traversalThread.reset(new TraversalDirThread(
                                 d->root->url(),QStringList(),
                                 QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System  | QDir::Hidden,
                                 QDirIterator::NoIteratorFlags));
    if (d->traversalThread.isNull()) {
        d->isUpdatedChildren = true;
        return;
    }
    QObject::connect(d->traversalThread.data(), &TraversalDirThread::updateChildren,
                     d.data(), &FileViewModelPrivate::doUpdateChildren,
                     Qt::QueuedConnection);

    if (d->canFetchMoreFlag) {
        d->canFetchMoreFlag = false;
        d->traversalThread->start();
    }
}

bool FileViewModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->canFetchMoreFlag;
}

void FileViewModel::updateViewItem(const QModelIndex &index)
{
    FileView *view = qobject_cast<FileView*>(qobject_cast<QObject*>(this)->parent());
    if (view) {
        view->update(index);
    }
}
DFMBASE_END_NAMESPACE
