// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rootinfo.h"
#include "fileitemdata.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/dfile.h>

#include <QApplication>
#include <QtConcurrent>
#include <QElapsedTimer>

using namespace dfmbase;
using namespace dfmplugin_workspace;

RootInfo::RootInfo(const QUrl &u, const bool canCache, QObject *parent)
    : QObject(parent), url(u), canCache(canCache)
{
    hiddenFileUrl.setScheme(url.scheme());
    hiddenFileUrl.setPath(DFMIO::DFMUtils::buildFilePath(url.path().toStdString().c_str(), ".hidden", nullptr));
}

RootInfo::~RootInfo()
{
    disconnect();
    if (watcher)
        watcher->stopWatcher();
    cancelWatcherEvent = true;
    watcherEventFuture.waitForFinished();
    for (const auto &thread : traversalThreads) {
        thread->traversalThread->stop();
        thread->traversalThread->wait();
    }
}

bool RootInfo::initThreadOfFileData(const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order, bool isMixFileAndFolder)
{
    // create traversal thread
    QSharedPointer<DirIteratorThread> traversalThread = traversalThreads.value(key);
    bool isGetCache = canCache;
    if (!traversalThread.isNull()) {
        traversalThread->traversalThread->disconnect();
    } else {
        isGetCache = (canCache && traversalFinish) || traversaling;
        if (canCache && traversalFinish && isRefresh)
            isGetCache = false;
    }

    traversalThread.reset(new DirIteratorThread);
    traversalThread->traversalThread.reset(
            new TraversalDirThreadManager(url, QStringList(),
                                          QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
                                          QDirIterator::FollowSymlinks));
    traversalThread->traversalThread->setSortAgruments(order, role, isMixFileAndFolder);
    traversalThread->traversalThread->setTraversalToken(key);
    initConnection(traversalThread->traversalThread);
    traversalThreads.insert(key, traversalThread);

    switch (role) {
    case Global::ItemRoles::kItemFileDisplayNameRole:
        traversalThread->originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileName;
        break;
    case Global::ItemRoles::kItemFileSizeRole:
        traversalThread->originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileSize;
        break;
    case Global::ItemRoles::kItemFileLastReadRole:
        traversalThread->originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastRead;
        break;
    case Global::ItemRoles::kItemFileLastModifiedRole:
        traversalThread->originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastModified;
        break;
    default:
        traversalThread->originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
    }
    traversalThread->originMixSort = isMixFileAndFolder;
    traversalThread->originSortOrder = order;
    return isGetCache;
}

void RootInfo::startWork(const QString &key, const bool getCache)
{
    if (!traversalThreads.contains(key))
        return;

    if (getCache)
        return handleGetSourceData(key);

    traversaling = true;
    {
        QWriteLocker lk(&childrenLock);
        childrenUrlList.clear();
        sourceDataList.clear();
    }
    traversalThreads.value(key)->traversalThread->start();
}

void RootInfo::startWatcher()
{
    if (needStartWatcher == false)
        return;
    needStartWatcher = false;
    if (watcher) {
        watcher->stopWatcher();
        watcher->disconnect(this);
    }

    // create watcher
    watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (watcher.isNull()) {
        fmWarning() << "Create watcher failed! url = " << url;
        return;
    }

    connect(watcher.data(), &AbstractFileWatcher::fileDeleted,
            this, &RootInfo::doFileDeleted);
    connect(watcher.data(), &AbstractFileWatcher::subfileCreated,
            this, &RootInfo::dofileCreated);
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
            this, &RootInfo::doFileUpdated);
    connect(watcher.data(), &AbstractFileWatcher::fileRename,
            this, &RootInfo::dofileMoved);

    watcher->restartWatcher();
}

int RootInfo::clearTraversalThread(const QString &key, const bool isRefresh)
{
    if (!traversalThreads.contains(key))
        return traversalThreads.count();

    auto thread = traversalThreads.take(key);
    auto traversalThread = thread->traversalThread;
    if (traversalThread->isRunning())
        emit traversalFinished(key);
    traversalThread->disconnect(this);
    if (traversalThread->isRunning()) {
        discardedThread.append(traversalThread);
        connect(
                thread->traversalThread.data(), &TraversalDirThread::finished, this, [this, traversalThread] {
                    discardedThread.removeAll(traversalThread);
                    traversalThread->disconnect();
                },
                Qt::QueuedConnection);
        traversaling = false;
    }
    thread->traversalThread->quit();
    if (traversalThreads.isEmpty())
        needStartWatcher = true;

    this->isRefresh = isRefresh;
    return traversalThreads.count();
}

void RootInfo::reset()
{
    {
        QWriteLocker lk(&childrenLock);
        childrenUrlList.clear();
        sourceDataList.clear();
    }

    traversaling = false;
    traversalFinish = false;
}

void RootInfo::doFileDeleted(const QUrl &url)
{
    enqueueEvent(QPair<QUrl, EventType>(url, kRmFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doThreadWatcherEvent), Qt::QueuedConnection);

    if (UniversalUtils::urlEquals(hiddenFileUrl, url))
        Q_EMIT watcherUpdateHideFile(url);
}

void RootInfo::dofileMoved(const QUrl &fromUrl, const QUrl &toUrl)
{
    Q_EMIT renameFileProcessStarted();
    doFileDeleted(fromUrl);

    AbstractFileInfoPointer info = InfoCacheController::instance().getCacheInfo(toUrl);
    if (info)
        info->refresh();

    dofileCreated(toUrl);

    // TODO(lanxs) TODO(xust) .hidden file's attribute changed signal not emitted in removable disks (vfat/exfat).
    // but renamed from a .goutputstream_xxx file
    // NOTE: GlobalEventType::kHideFiles event is watched in fileview, but this can be used to notify update view
    // when the file is modified in other way.
    if (UniversalUtils::urlEquals(hiddenFileUrl, toUrl))
        Q_EMIT watcherUpdateHideFile(toUrl);
}

void RootInfo::dofileCreated(const QUrl &url)
{
    enqueueEvent(QPair<QUrl, EventType>(url, kAddFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doThreadWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::doFileUpdated(const QUrl &url)
{
    enqueueEvent(QPair<QUrl, EventType>(url, kUpdateFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doThreadWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::doWatcherEvent()
{
    if (processFileEventRuning)
        return;

    processFileEventRuning = true;
    QElapsedTimer timer;
    timer.start();
    QList<QUrl> adds, updates, removes;
    qint64 oldtime = 0;
    int emptyLoopCount = 0;
    while (checkFileEventQueue() || timer.elapsed() < 200) {
        // 检查超时，重新设置起始时间
        if (timer.elapsed() - oldtime >= 200) {
            // 处理添加文件
            if (!adds.isEmpty())
                addChildren(adds);
            if (!updates.isEmpty())
                updateChildren(updates);
            if (!removes.isEmpty())
                removeChildren(removes);

            adds.clear();
            updates.clear();
            removes.clear();

            oldtime = timer.elapsed();
        }

        if (cancelWatcherEvent)
            return;

        if (!checkFileEventQueue()) {
            if (emptyLoopCount >= 5)
                break;

            QThread::msleep(10);
            if (adds.isEmpty() && updates.isEmpty() && removes.isEmpty())
                oldtime = timer.elapsed();

            ++emptyLoopCount;
            continue;
        }

        emptyLoopCount = 0;
        QPair<QUrl, EventType> event = dequeueEvent();
        const QUrl &fileUrl = event.first;

        if (cancelWatcherEvent)
            return;

        if (!fileUrl.isValid())
            continue;

        if (UniversalUtils::urlEquals(fileUrl, url)) {
            if (event.second == kAddFile)
                continue;
            else if (event.second == kRmFile) {
                emit InfoCacheController::instance().removeCacheFileInfo({ fileUrl });
                WatcherCache::instance().removeCacheWatcherByParent(fileUrl);
                emit requestCloseTab(fileUrl);
                emit requestClearRoot(fileUrl);
                QWriteLocker lk(&childrenLock);
                childrenUrlList.clear();
                sourceDataList.clear();
                break;
            }
        }

        if (cancelWatcherEvent)
            return;

        if (event.second == kAddFile) {
            // 判断update和remove中是否存在存在就移除
            updates.removeOne(fileUrl);
            removes.removeOne(fileUrl);
            if (adds.contains(fileUrl))
                continue;

            adds.append(fileUrl);
        } else if (event.second == kUpdateFile) {
            // 如果在增加或者移除中就不添加
            if (adds.contains(fileUrl) || removes.contains(fileUrl) || updates.contains(fileUrl))
                continue;
            updates.append(fileUrl);
        } else {
            adds.removeOne(fileUrl);
            updates.removeOne(fileUrl);
            if (removes.contains(fileUrl))
                continue;
            removes.append(fileUrl);
        }
    }

    // 处理添加文件
    if (!removes.isEmpty())
        removeChildren(removes);
    if (!adds.isEmpty())
        addChildren(adds);
    if (!updates.isEmpty())
        updateChildren(updates);
    processFileEventRuning = false;
}

void RootInfo::doThreadWatcherEvent()
{
    if (processFileEventRuning)
        return;

    watcherEventFuture = QtConcurrent::run([&]() {
        if (cancelWatcherEvent)
            return;
        doWatcherEvent();
    });
}

void RootInfo::handleTraversalResult(const FileInfoPointer &child, const QString &travseToken)
{
    auto sortInfo = addChild(child);
    if (sortInfo)
        Q_EMIT iteratorAddFile(travseToken, sortInfo, child);
}

void RootInfo::handleTraversalResults(const QList<FileInfoPointer> children, const QString &travseToken)
{
    QList<SortInfoPointer> sortInfos;
    QList<FileInfoPointer> infos;
    for (const auto &info : children) {
        auto sortInfo = addChild(info);
        if (!sortInfo)
            continue;
        sortInfos.append(sortInfo);

        infos.append(info);
    }

    if (sortInfos.length() > 0)
        Q_EMIT iteratorAddFiles(travseToken, sortInfos, infos);
}

void RootInfo::handleTraversalLocalResult(QList<SortInfoPointer> children,
                                          dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                                          Qt::SortOrder sortOrder, bool isMixDirAndFile, const QString &travseToken)
{
    originSortRole = sortRole;
    originSortOrder = sortOrder;
    originMixSort = isMixDirAndFile;

    addChildren(children);
    traversaling = false;

    Q_EMIT iteratorLocalFiles(travseToken, children, originSortRole, originSortOrder, originMixSort);
}

void RootInfo::handleTraversalFinish(const QString &travseToken)
{
    traversaling = false;
    emit traversalFinished(travseToken);
    traversalFinish = true;
    if (isRefresh) {
        isRefresh = false;
    }
}

void RootInfo::handleTraversalSort(const QString &travseToken)
{
    emit requestSort(travseToken, url);
}

void RootInfo::handleGetSourceData(const QString &currentToken)
{
    if (needStartWatcher)
        startWatcher();

    QList<SortInfoPointer> newDatas;
    {
        QWriteLocker wlk(&childrenLock);
        newDatas = sourceDataList;
    }

    emit sourceDatas(currentToken, newDatas, originSortRole, originSortOrder, originMixSort, !traversaling);
    if (!traversaling)
        emit traversalFinished(currentToken);
}

void RootInfo::initConnection(const TraversalThreadManagerPointer &traversalThread)
{
    connect(traversalThread.data(), &TraversalDirThreadManager::updateChildrenManager,
            this, &RootInfo::handleTraversalResults, Qt::DirectConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::updateLocalChildren,
            this, &RootInfo::handleTraversalLocalResult, Qt::DirectConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::traversalRequestSort,
            this, &RootInfo::handleTraversalSort, Qt::DirectConnection);
    // 主线中执行
    connect(traversalThread.data(), &TraversalDirThreadManager::traversalFinished,
            this, &RootInfo::handleTraversalFinish, Qt::QueuedConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::iteratorInitFinished,
            this, &RootInfo::startWatcher, Qt::QueuedConnection);
}

void RootInfo::addChildren(const QList<QUrl> &urlList)
{
    QList<SortInfoPointer> newSortInfo;

    for (auto url : urlList) {
        url.setPath(url.path());

        auto child = fileInfo(url);

        if (!child)
            continue;

        auto sortInfo = addChild(child);
        if (sortInfo)
            newSortInfo.append(sortInfo);
    }

    if (newSortInfo.count() > 0) {
        originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
        emit watcherAddFiles(newSortInfo);
    }
}

void RootInfo::addChildren(const QList<FileInfoPointer> &children)
{
    for (auto &child : children) {
        addChild(child);
    }
}

void RootInfo::addChildren(const QList<SortInfoPointer> &children)
{
    for (auto &file : children) {
        if (!file)
            continue;

        QWriteLocker lk(&childrenLock);
        childrenUrlList.append(file->fileUrl());
        sourceDataList.append(file);
    }
}

SortInfoPointer RootInfo::addChild(const FileInfoPointer &child)
{
    if (!child)
        return nullptr;

    QUrl childUrl = child->urlOf(dfmbase::UrlInfoType::kUrl);
    childUrl.setPath(childUrl.path());

    SortInfoPointer sort = sortFileInfo(child);
    if (!sort)
        return nullptr;

    {
        QWriteLocker lk(&childrenLock);
        if (childrenUrlList.contains(childUrl)) {
            sourceDataList.replace(childrenUrlList.indexOf(childUrl), sort);
            return sort;
        }
        childrenUrlList.append(childUrl);
        sourceDataList.append(sort);
    }

    return sort;
}

SortInfoPointer RootInfo::sortFileInfo(const FileInfoPointer &info)
{
    if (!info)
        return nullptr;
    SortInfoPointer sortInfo(new SortFileInfo);
    sortInfo->setUrl(info->urlOf(UrlInfoType::kUrl));
    sortInfo->setSize(info->size());
    sortInfo->setFile(!info->isAttributes(OptInfoType::kIsDir));
    sortInfo->setDir(info->isAttributes(OptInfoType::kIsDir));
    sortInfo->setHide(info->isAttributes(OptInfoType::kIsHidden));
    sortInfo->setSymlink(info->isAttributes(OptInfoType::kIsSymLink));
    sortInfo->setReadable(info->isAttributes(OptInfoType::kIsReadable));
    sortInfo->setWriteable(info->isAttributes(OptInfoType::kIsWritable));
    sortInfo->setExecutable(info->isAttributes(OptInfoType::kIsExecutable));
    return sortInfo;
}

void RootInfo::removeChildren(const QList<QUrl> &urlList)
{
    QList<SortInfoPointer> removeChildren {};
    int childIndex = -1;
    QList<QUrl> removeUrls;
    emit InfoCacheController::instance().removeCacheFileInfo(urlList);
    for (QUrl url : urlList) {
        WatcherCache::instance().removeCacheWatcherByParent(url);
        emit requestCloseTab(url);
        url.setPath(url.path());
        auto child = fileInfo(url);
        if (!child)
            continue;

        auto realUrl = child->urlOf(UrlInfoType::kUrl);
        removeUrls.append(realUrl);
        QWriteLocker lk(&childrenLock);
        childIndex = childrenUrlList.indexOf(realUrl);
        if (childIndex < 0 || childIndex >= childrenUrlList.length()) {
            removeChildren.append(sortFileInfo(child));
            continue;
        }
        childrenUrlList.removeAt(childIndex);
        removeChildren.append(sourceDataList.takeAt(childIndex));
    }

    if (removeUrls.count() > 0)
        emit InfoCacheController::instance().removeCacheFileInfo(removeUrls);

    if (removeChildren.count() > 0)
        emit watcherRemoveFiles(removeChildren);
}

bool RootInfo::containsChild(const QUrl &url)
{
    QReadLocker lk(&childrenLock);
    return childrenUrlList.contains(url);
}

SortInfoPointer RootInfo::updateChild(const QUrl &url)
{
    SortInfoPointer sort { nullptr };

    auto info = fileInfo(url);
    if (info.isNull())
        return nullptr;

    auto realUrl = info->urlOf(UrlInfoType::kUrl);

    QWriteLocker lk(&childrenLock);
    if (!childrenUrlList.contains(realUrl))
        return nullptr;
    sort = sortFileInfo(info);
    if (sort.isNull())
        return nullptr;
    sourceDataList.replace(childrenUrlList.indexOf(realUrl), sort);

    // NOTE: GlobalEventType::kHideFiles event is watched in fileview, but this can be used to notify update view
    // when the file is modified in other way.
    if (UniversalUtils::urlEquals(hiddenFileUrl, url))
        Q_EMIT watcherUpdateHideFile(url);

    return sort;
}

void RootInfo::updateChildren(const QList<QUrl> &urls)
{
    QList<SortInfoPointer> updates;
    for (auto url : urls) {
        auto sort = updateChild(url);
        if (sort)
            updates.append(sort);
    }
    if (updates.isEmpty())
        return;
    emit watcherUpdateFiles(updates);
}

bool RootInfo::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    return !watcherEvent.isEmpty();
}

void RootInfo::enqueueEvent(const QPair<QUrl, EventType> &e)
{
    QMutexLocker lk(&watcherEventMutex);
    watcherEvent.enqueue(e);
}

QPair<QUrl, RootInfo::EventType> RootInfo::dequeueEvent()
{
    QMutexLocker lk(&watcherEventMutex);
    if (!watcherEvent.isEmpty())
        return watcherEvent.dequeue();

    return QPair<QUrl, RootInfo::EventType>();
}

// When monitoring the mtp directory, the monitor monitors that the scheme of the
// url used for adding and deleting files is mtp (mtp://path).
// Here, the monitor's url is used to re-complete the current url
FileInfoPointer RootInfo::fileInfo(const QUrl &url)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!info.isNull())
        return info;

    if (!watcher)
        return nullptr;

    const QUrl &parentUrl = QUrl::fromPercentEncoding(watcher->url().toString().toUtf8());
    auto path = url.path();
    if (path.isEmpty() || path == QDir::separator() || url.fileName().isEmpty())
        return info;

    auto pathParent = path.endsWith(QDir::separator()) ? path.left(path.length() - 1) : path;
    auto parentPath = parentUrl.path().endsWith(QDir::separator())
            ? parentUrl.path().left(parentUrl.path().length() - 1)
            : parentUrl.path();
    pathParent = pathParent.left(pathParent.lastIndexOf(QDir::separator()));
    if (!parentPath.endsWith(pathParent.mid(1)))
        return info;

    auto currentUrl = parentUrl;
    currentUrl.setPath(currentUrl.path(QUrl::PrettyDecoded) + QDir::separator() + url.fileName());
    info = InfoFactory::create<FileInfo>(currentUrl);
    return info;
}
