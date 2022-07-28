/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "utils/fileoperatorhelper.h"
#include "events/workspaceeventsequence.h"
#include "filesortfilterproxymodel.h"

#include "base/application/settings.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/base/application/application.h"

#include <dfm-framework/event/event.h>

#include <QApplication>
#include <QPointer>
#include <QList>
#include <QMimeData>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewModelPrivate::FileViewModelPrivate(FileViewModel *qq)
    : QObject(qq), q(qq)
{
    nodeManager.reset(new FileNodeManagerThread(qq));

    updateTimer.setInterval(15);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, &QTimer::timeout, qq, &FileViewModel::onFilesUpdated);
}

FileViewModelPrivate::~FileViewModelPrivate()
{
}

// 文件在当前目录下创建、修改、删除都需要顺序处理，并且都是在遍历目录完成后才有序的处理
// 处理方式，收到这3个事件，都加入到事件处理队列
// 判断当前遍历目录是否完成，是-启动异步文件监视事件处理，否-文件遍历完成时，启动异步文件监视事件处理
// 在退出时清理异步事件处理，再次进行fetchmore时（遍历新目录时）清理文件监视事件。
void FileViewModelPrivate::doFileDeleted(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl, EventType>(url, kRmFile));
    }
    //    if (isUpdatedChildren)
    //        return;
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileViewModelPrivate::dofileMoved(const QUrl &fromUrl, const QUrl &toUrl)
{
    doFileDeleted(fromUrl);

    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(toUrl);
    if (info)
        info->refresh();

    dofileCreated(toUrl);
}

void FileViewModelPrivate::dofileCreated(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl, EventType>(url, kAddFile));
    }
    // Todo(liyigang):isUpdatedChildren
    //    if (isUpdatedChildren)
    //        return;

    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileViewModelPrivate::doFileUpdated(const QUrl &url)
{
    //     Todo(yanghao): filter .hidden file update
    if (!updateUrlList.containsByLock(url))
        updateUrlList.appendByLock(url);

    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info)
        info->refresh();

    q->dataChanged(q->findIndex(url), q->findIndex(url));

    if (!updateTimer.isActive())
        updateTimer.start();
}

void FileViewModelPrivate::doFilesUpdated()
{
}

void FileViewModelPrivate::doWatcherEvent()
{
    // Todo(liyigang):isUpdatedChildren
    //    if (isUpdatedChildren)
    //        return;

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

        if (!fileUrl.isValid())
            continue;

        if (UniversalUtils::urlEquals(fileUrl, rootData->url())) {
            if (event.second == kAddFile)
                continue;
            else if (event.second == kRmFile) {
                nodeManager->clearChildren();
                dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Close", fileUrl);
                break;
            }
            //todo:先不做
        }

        // Todo(liyigang): parentUrl
        // if (UrlRoute::urlParent(fileUrl) != root->url())
        //    continue;

        if (event.second == kAddFile) {
            nodeManager->insertChild(fileUrl);
            q->selectAndRenameFile(fileUrl);
        } else {
            nodeManager->removeChildren(fileUrl);
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Close", fileUrl);
        }
    }
    q->childrenUpdated();
    processFileEventRuning = false;
}

bool FileViewModelPrivate::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    bool isEmptyQueue = watcherEvent.isEmpty();
    return !isEmptyQueue;
}

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel(parent), d(new FileViewModelPrivate(this))
{
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::requestFileUpdate, this, &FileViewModel::onFileUpdated);
    d->view = dynamic_cast<FileView *>(parent);
}

FileViewModel::~FileViewModel()
{
    clear();
}

QModelIndex FileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() || row < 0 || column < 0 || d->nodeManager->childrenCount() <= row)
        return rootIndex();

    return createIndex(row, column, d->nodeManager->childByIndex(row).data());
}

const FileViewItem *FileViewModel::itemFromIndex(const QModelIndex &index) const
{
    if (index == rootIndex())
        return d->rootData.data();

    FileNodePointer child = d->nodeManager->childByIndex(index.row());
    if (child.isNull())
        return nullptr;

    return child.data();
}

QUrl FileViewModel::rootUrl() const
{
    if (d->rootData)
        return d->rootData->url();

    return QUrl();
}

QModelIndex FileViewModel::rootIndex() const
{
    return createIndex(0, 0, d->rootData.data());
}

const FileViewItem *FileViewModel::rootItem() const
{
    return d->rootData.data();
}

void FileViewModel::setRootUrl(const QUrl &url)
{
    if (url.scheme() == Scheme::kFile && d->rootData && rootUrl() == url) {
        QApplication::restoreOverrideCursor();
        return;
    }

    clear();

    if (!url.isValid())
        return;

    // insert root index
    beginInsertRows(QModelIndex(), 0, 0);
    d->rootData.reset(new FileViewItem(nullptr, url));
    d->nodeManager->setRootNode(d->rootData);
    insertRow(0, QModelIndex());
    endInsertRows();

    if (!d->watcher.isNull()) {
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileDeleted,
                   d.data(), &FileViewModelPrivate::doFileDeleted);
        disconnect(d->watcher.data(), &AbstractFileWatcher::subfileCreated,
                   d.data(), &FileViewModelPrivate::dofileCreated);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                   d.data(), &FileViewModelPrivate::doFileUpdated);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileRename,
                   d.data(), &FileViewModelPrivate::dofileMoved);
        d->watcher->stopWatcher();
    }
    d->watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (!d->watcher.isNull()) {
        connect(d->watcher.data(), &AbstractFileWatcher::fileDeleted,
                d.data(), &FileViewModelPrivate::doFileDeleted);
        connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated,
                d.data(), &FileViewModelPrivate::dofileCreated);
        connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                d.data(), &FileViewModelPrivate::doFileUpdated);
        connect(d->watcher.data(), &AbstractFileWatcher::fileRename,
                d.data(), &FileViewModelPrivate::dofileMoved);

        d->watcher->startWatcher();
    }

    d->canFetchMoreFlag = true;

    fetchMore(rootIndex());
}

QModelIndex FileViewModel::findIndex(const QUrl &url) const
{
    if (url.isValid()) {
        QPair<int, FileNodePointer> pair = d->nodeManager->childByUrl(url);
        if (pair.first >= 0)
            return createIndex(pair.first, 0, pair.second.data());
    }

    return QModelIndex();
}

AbstractFileInfoPointer FileViewModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    if (index.row() < 0 || d->nodeManager->childrenCount() <= index.row())
        return nullptr;
    FileNodePointer child = d->nodeManager->childByIndex(index.row());
    if (child.isNull())
        return nullptr;
    return child->fileInfo();
}

QModelIndex FileViewModel::parent(const QModelIndex &child) const
{
    FileViewItem *childItem = static_cast<FileViewItem *>(child.internalPointer());
    if (!childItem || childItem->parent != d->nodeManager->rootNode().data())
        return QModelIndex();

    return createIndex(0, 0, childItem->parent);
}

int FileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->nodeManager->childrenCount();
}

int FileViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return getColumnRoles().count();
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    auto item = itemFromIndex(index);
    if (item) {
        QVariant data;
        if (WorkspaceEventSequence::instance()->doFetchCustomRoleData(rootUrl(), item->url(), static_cast<ItemRoles>(role), &data))
            return data;
        return item->data(role);
    }

    return QVariant();
}

void FileViewModel::clear()
{
    d->nodeManager->disconnect(d->nodeManager.data());
    d->nodeManager->stop();

    // remove children
    beginRemoveRows(rootIndex(), 0, d->nodeManager->childrenCount() - 1);
    removeRows(0, d->nodeManager->childrenCount(), rootIndex());
    d->nodeManager->clearChildren();
    endRemoveRows();

    // remove root
    beginRemoveRows(QModelIndex(), 0, 0);
    removeRows(0, 1, QModelIndex());
    d->rootData.clear();
    endRemoveRows();
}

void FileViewModel::update()
{
    if (0 == d->nodeManager->childrenCount())
        return;

    for (int i = 0; i < d->nodeManager->childrenCount(); ++i) {
        auto child = d->nodeManager->childByIndex(i);
        if (Q_UNLIKELY(!child))
            continue;
        child->refresh();
    }

    emit dataChanged(index(0, 0, rootIndex()), index(d->nodeManager->childrenCount(), 0, rootIndex()));
}

void FileViewModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent))
        return;

    d->canFetchMoreFlag = false;

    auto url = rootUrl();
    auto prehandler = WorkspaceHelper::instance()->viewRoutePrehandler(url.scheme());
    if (prehandler) {
        QPointer<FileViewModel> guard(this);
        quint64 winId = DFMBASE_NAMESPACE::FileManagerWindowsManager::instance().findWindowId(d->view);
        prehandler(winId, url, [=]() { if (guard) this->traversCurrDir(); });
    } else {
        traversCurrDir();
    }
}

bool FileViewModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->canFetchMoreFlag;
}

Qt::ItemFlags FileViewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    const FileViewItem *item = itemFromIndex(index);
    if (!item)
        return flags;

    const AbstractFileInfoPointer &fileInfo = item->fileInfo();
    if (fileInfo && fileInfo->canRename()) {
        flags |= Qt::ItemIsEditable;
    }
    if (fileInfo && fileInfo->isWritable()) {
        if (fileInfo->canDrop())
            flags |= Qt::ItemIsDropEnabled;
        else
            flags |= Qt::ItemNeverHasChildren;
    }
    if (fileInfo && fileInfo->canDrag())
        flags |= Qt::ItemIsDragEnabled;

    return flags;
}

QStringList FileViewModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

QMimeData *FileViewModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QSet<QUrl> urlsSet;
    QList<QModelIndex>::const_iterator it = indexes.begin();

    for (; it != indexes.end(); ++it) {
        if ((*it).column() == 0) {
            const AbstractFileInfoPointer &fileInfo = this->fileInfo(*it);
            const QUrl &url = fileInfo->url();

            if (urlsSet.contains(url))
                continue;

            urls << url;
            urlsSet << url;
        }
    }

    QMimeData *data = new QMimeData();
    data->setUrls(urls);

    QByteArray userID;
    userID.append(QString::number(SysInfoUtils::getUserId()));
    data->setData(DFMGLOBAL_NAMESPACE::Mime::kMimeDataUserIDKey, userID);

    return data;
}

bool FileViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    const QModelIndex &dropIndex = index(row, column, parent);

    if (!dropIndex.isValid())
        return false;

    QUrl targetUrl = itemFromIndex(dropIndex)->url();
    AbstractFileInfoPointer targetFileInfo = itemFromIndex(dropIndex)->fileInfo();
    QList<QUrl> dropUrls = data->urls();
    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", dropUrls, &urls);
    if (ok && !urls.isEmpty())
        dropUrls = urls;

    if (targetFileInfo->isSymLink())
        targetUrl = QUrl::fromLocalFile(targetFileInfo->symLinkTarget());

    FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());

    if (FileUtils::isTrashDesktopFile(targetUrl)) {
        FileOperatorHelperIns->moveToTrash(view, dropUrls);
        return true;
    } else if (FileUtils::isDesktopFile(targetUrl)) {
        FileOperatorHelperIns->openFilesByApp(view, dropUrls, QStringList { targetUrl.toLocalFile() });
        return true;
    }

    bool ret { true };

    switch (action) {
    case Qt::CopyAction:
        if (dropUrls.count() > 0) {
            // call copy
            FileOperatorHelperIns->dropFiles(view, Qt::CopyAction, targetUrl, dropUrls);
        }
        break;
    case Qt::MoveAction:
        if (dropUrls.count() > 0) {
            // call move
            FileOperatorHelperIns->dropFiles(view, Qt::MoveAction, targetUrl, dropUrls);
        }
        break;
    default:
        break;
    }

    return ret;
}

Qt::DropActions FileViewModel::supportedDragActions() const
{
    if (d->rootData && d->rootData->fileInfo())
        return d->rootData->fileInfo()->supportedDragActions();

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions FileViewModel::supportedDropActions() const
{
    if (d->rootData && d->rootData->fileInfo())
        return d->rootData->fileInfo()->supportedDropActions();

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

void FileViewModel::beginInsertRows(const QModelIndex &parent, int first, int last)
{
    QAbstractItemModel::beginInsertRows(parent, first, last);
}

void FileViewModel::endInsertRows()
{
    QAbstractItemModel::endInsertRows();
}

void FileViewModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    QAbstractItemModel::beginRemoveRows(parent, first, last);
}

void FileViewModel::endRemoveRows()
{
    QAbstractItemModel::endRemoveRows();
}

AbstractFileWatcherPointer FileViewModel::fileWatcher() const
{
    return d->watcher;
}

FileViewModel::State FileViewModel::state() const
{
    return d->currentState;
}

void FileViewModel::setState(FileViewModel::State state)
{
    if (d->currentState == state)
        return;

    d->currentState = state;

    emit stateChanged();
}

void FileViewModel::childrenUpdated()
{
    emit modelChildrenUpdated();
}

void FileViewModel::traversCurrDir()
{
    d->isUpdatedChildren = false;
    if (!d->traversalThread.isNull()) {
        d->traversalThread.data()->disconnect();
        d->traversalThread->stopAndDeleteLater();
        d->traversalThread->setParent(nullptr);
    }

    d->traversalThread = new TraversalDirThread(
            rootUrl(), QStringList(),
            QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
            QDirIterator::NoIteratorFlags);

    d->traversalThread->setParent(this);

    if (d->traversalThread.isNull()) {
        d->isUpdatedChildren = true;
        return;
    }

    d->nodeManager->connect(d->traversalThread.data(), &TraversalDirThread::updateChild,
                            d->nodeManager.data(), &FileNodeManagerThread::onHandleAddFile,
                            Qt::QueuedConnection);
    d->nodeManager->connect(d->traversalThread.data(), &QThread::finished,
                            d->nodeManager.data(), &FileNodeManagerThread::onHandleTraversalFinished,
                            Qt::QueuedConnection);

    setState(Busy);
    d->traversalThread->start();
}

void FileViewModel::stopTraversWork()
{
    if (d->traversalThread)
        d->traversalThread->stop();
}

void FileViewModel::selectAndRenameFile(const QUrl &fileUrl)
{
    FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());
    if (!view)
        return;

    quint64 windowId = WorkspaceHelper::instance()->windowId(view);

    if (WorkspaceHelper::kSelectionAndRenameFile.contains(windowId)) {
        if (WorkspaceHelper::kSelectionAndRenameFile[windowId].first == rootUrl()) {
            WorkspaceHelper::kSelectionAndRenameFile[windowId] = qMakePair(QUrl(), QUrl());

            QTimer::singleShot(100, this, [=] {
                emit selectAndEditFile(fileUrl);
            });
        }
    }
}

QList<ItemRoles> FileViewModel::getColumnRoles() const
{
    QList<ItemRoles> roles;
    bool customOnly = WorkspaceEventSequence::instance()->doFetchCustomColumnRoles(rootUrl(), &roles);

    const QVariantMap &map = DFMBASE_NAMESPACE::Application::appObtuselySetting()->value("FileViewState", rootUrl()).toMap();
    if (map.contains("headerList")) {
        QVariantList headerList = map.value("headerList").toList();

        for (ItemRoles role : roles) {
            if (!headerList.contains(role))
                headerList.append(role);
        }

        roles.clear();
        for (auto var : headerList) {
            roles.append(static_cast<ItemRoles>(var.toInt()));
        }
    } else if (!customOnly) {
        static QList<ItemRoles> defualtColumnRoleList = QList<ItemRoles>() << kItemFileDisplayNameRole
                                                                           << kItemFileLastModifiedRole
                                                                           << kItemFileSizeRole
                                                                           << kItemFileMimeTypeRole;

        int customCount = roles.count();
        for (auto role : defualtColumnRoleList) {
            if (!roles.contains(role))
                roles.insert(roles.length() - customCount, role);
        }
    }

    return roles;
}

void FileViewModel::onFilesUpdated()
{
    FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());
    if (view) {
        QDir::Filters filter = view->model()->getFilters();
        view->model()->setFilters(filter);
    }
    emit updateFiles();
}

void FileViewModel::onFileUpdated(const QUrl &url)
{
    const QModelIndex &index = findIndex(url);
    if (index.isValid()) {
        auto info = InfoFactory::create<AbstractFileInfo>(url);
        if (info)
            info->refresh();

        emit dataChanged(index, index);
    }
}

FileNodeManagerThread::FileNodeManagerThread(FileViewModel *parent)
    : QThread(parent)
{
}

FileNodeManagerThread::~FileNodeManagerThread()
{
    stop();
}

void FileNodeManagerThread::onHandleAddFile(const QUrl url)
{
    {
        QMutexLocker lk(&fileQueueMutex);
        fileQueue.enqueue(url);
    }
    if (!isRunning()) {
        stoped = false;
        start();
    }
}

void FileNodeManagerThread::onHandleTraversalFinished()
{
    isTraversalFinished = true;
    QApplication::restoreOverrideCursor();

    if (!isRunning())
        model()->setState(FileViewModel::Idle);
}

void FileNodeManagerThread::stop()
{
    {
        QMutexLocker lk(&fileQueueMutex);
        fileQueue.clear();
    }
    stoped = true;
    wait();
    isTraversalFinished = false;
    stoped = false;

    model()->setState(FileViewModel::Idle);
}

void FileNodeManagerThread::clearChildren()
{
    childrenMutex.lock();

    model()->beginRemoveRows(model()->rootIndex(), 0, childrenUrlList.count());
    model()->removeRows(0, childrenUrlList.count(), model()->rootIndex());
    children.clear();
    childrenUrlList.clear();
    model()->endRemoveRows();

    childrenMutex.unlock();
}

FileNodePointer FileNodeManagerThread::rootNode() const
{
    return root;
}

void FileNodeManagerThread::insertChild(const QUrl &url)
{
    int row = -1;
    {
        QMutexLocker lk(&childrenMutex);
        if (children.contains(url))
            return;
        row = children.count();
    }

    model()->beginInsertRows(model()->rootIndex(), row, row);
    FileNodePointer item(new FileViewItem(root.data(), url));
    {
        QMutexLocker lk(&childrenMutex);
        childrenUrlList.append(url);
        children.insert(url, item);
        model()->insertRow(row, model()->rootIndex());
    }
    model()->endInsertRows();
}

bool FileNodeManagerThread::insertChildren(QList<QUrl> &urls)
{
    int row = -1;

    for (const auto &url : urls) {
        if (stoped)
            return false;

        FileNodePointer needNode(new FileViewItem(root.data(), url));
        const AbstractFileInfoPointer &needNodeInfo = needNode->fileInfo();
        if (needNodeInfo.isNull())
            continue;

        row = childrenCount();

        model()->beginInsertRows(model()->rootIndex(), row, row);

        QMutexLocker lk(&childrenMutex);
        childrenUrlList.append(url);
        children.insert(url, needNode);
        model()->insertRow(row, model()->rootIndex());

        model()->endInsertRows();
    }

    urls.clear();

    return !stoped;
}

void FileNodeManagerThread::removeChildren(const QUrl &url)
{
    int fileIndex = -1;
    {
        QMutexLocker lk(&childrenMutex);
        fileIndex = childrenUrlList.indexOf(url);
    }

    if (fileIndex == -1)
        return;

    model()->beginRemoveRows(model()->rootIndex(), fileIndex, fileIndex);
    {
        QMutexLocker lk(&childrenMutex);
        childrenUrlList.removeOne(url);
        children.remove(url);
        model()->removeRow(fileIndex, model()->rootIndex());
    }
    model()->endRemoveRows();
}

int FileNodeManagerThread::childrenCount()
{
    QMutexLocker lk(&childrenMutex);
    return children.count();
}

FileNodePointer FileNodeManagerThread::childByIndex(const int &index)
{
    FileNodePointer child { nullptr };

    QMutexLocker lk(&childrenMutex);
    if (index >= 0 && index < childrenUrlList.size()) {
        const QUrl &url = childrenUrlList.at(index);
        if (children.contains(url))
            child = children[childrenUrlList.at(index)];
    }
    return child;
}

QPair<int, FileNodePointer> FileNodeManagerThread::childByUrl(const QUrl &url)
{
    QMutexLocker lk(&childrenMutex);
    if (children.contains(url)) {
        FileNodePointer node = children[url];
        int index = childrenUrlList.indexOf(url);
        return QPair<int, FileNodePointer>(index, node);
    }

    return QPair<int, FileNodePointer>(-1, nullptr);
}

bool FileNodeManagerThread::fileQueueEmpty()
{
    QMutexLocker lk(&fileQueueMutex);
    return fileQueue.isEmpty();
}

int FileNodeManagerThread::fileQueueCount()
{
    QMutexLocker lk(&fileQueueMutex);
    return fileQueue.count();
}

QUrl FileNodeManagerThread::dequeueFileQueue()
{
    QMutexLocker lk(&fileQueueMutex);
    if (fileQueue.isEmpty())
        return QUrl();
    return fileQueue.dequeue();
}

void FileNodeManagerThread::run()
{
    if (stoped)
        return;

    if (!insertChildrenByCeiled()) {
        model()->setState(FileViewModel::Idle);
        return;
    }

    QUrl url;

    QList<QUrl> needInsertList;

    while (!fileQueueEmpty() || !isTraversalFinished) {
        if (stoped)
            return;

        url = dequeueFileQueue();
        if (!url.isValid())
            continue;

        if (!isTraversalFinished) {
            insertChild(url);
        } else if (isTraversalFinished && !fileQueueEmpty()) {
            needInsertList.append(url);
        } else {
            needInsertList.append(url);
            if (!insertChildren(needInsertList)) {
                model()->setState(FileViewModel::Idle);
                return;
            }
        }
    }

    if (needInsertList.count() > 0) {
        insertChildren(needInsertList);
    }

    model()->setState(FileViewModel::Idle);
}

bool FileNodeManagerThread::insertChildrenByCeiled()
{
    if (stoped)
        return false;
    QTime ceilTime;
    ceilTime.start();
    QList<QUrl> files;
    while (!isTraversalFinished && ceilTime.elapsed() < timeCeiling && files.count() < countCeiling) {
        if (stoped)
            return false;
        QUrl file = dequeueFileQueue();
        if (file.isValid())
            files << file;
    }

    return insertChildren(files);
}
