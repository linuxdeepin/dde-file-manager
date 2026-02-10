// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rootinfo.h"
#include "fileitemdata.h"
#include "utils/keywordextractor.h"

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
    fmInfo() << "RootInfo created for URL:" << url.toString() << "canCache:" << canCache;

    // 使用关键字提取器来处理关键字解析
    keyWords = KeywordExtractorManager::instance().extractor().extractFromUrl(url);

    if (!keyWords.isEmpty()) {
        fmDebug() << "Extracted keywords for search:" << keyWords;
    }

    hiddenFileUrl.setScheme(url.scheme());
    hiddenFileUrl.setPath(DFMIO::DFMUtils::buildFilePath(url.path().toStdString().c_str(), ".hidden", nullptr));
}

RootInfo::~RootInfo()
{
    isDying.store(true);
    fmInfo() << "RootInfo destructor started for URL:" << url.toString();

    disconnect();
    if (watcher) {
        fmDebug() << "Stopping file watcher";
        watcher->stopWatcher();
    }

    cancelWatcherEvent = true;
    for (auto &future : watcherEventFutures) {
        future.waitForFinished();
    }

    fmDebug() << "Stopping" << traversalThreads.size() << "traversal threads";
    for (const auto &thread : traversalThreads) {
        thread->traversalThread->stop();
        thread->traversalThread->wait();
    }

    // wait old dir iterator thread
    for (const auto &thread : discardedThread) {
        thread->disconnect();
        thread->stop();
        thread->quit();
        thread->wait();
    }

    fmInfo() << "RootInfo destructor completed for URL:" << url.toString();
}

bool RootInfo::initThreadOfFileData(const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order, bool isMixFileAndFolder)
{
    fmDebug() << "Initializing file data thread for key:" << key << "role:" << role
              << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending")
              << "mixFileAndFolder:" << isMixFileAndFolder;

    // clear old dir iterator thread
    for (auto it = discardedThread.begin(); it != discardedThread.end();) {
        if (!(*it)->isRunning()) {
            it = discardedThread.erase(it);
        } else {
            it++;
        }
    }

    // create traversal thread
    QSharedPointer<DirIteratorThread> traversalThread = traversalThreads.value(key);
    bool isGetCache = canCache;
    if (!traversalThread.isNull()) {
        fmDebug() << "Disconnecting existing traversal thread for key:" << key;
        traversalThread->traversalThread->disconnect();
    } else {
        isGetCache = (canCache && traversalFinish) || traversaling;
        if (canCache && traversalFinish && isRefresh) {
            isGetCache = false;
            fmDebug() << "Cache disabled due to refresh flag";
        }
    }

    fmDebug() << "Creating new traversal thread for URL:" << url.toString() << "isGetCache:" << isGetCache;

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
    if (!traversalThreads.contains(key)) {
        fmWarning() << "Cannot start work: traversal thread not found for key:" << key;
        return;
    }

    fmDebug() << "Starting work for key:" << key << "getCache:" << getCache;

    if (getCache) {
        fmDebug() << "Using cached data for key:" << key;
        return handleGetSourceData(key);
    }

    fmInfo() << "Starting directory traversal for URL:" << url.toString();
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
    if (needStartWatcher == false) {
        fmDebug() << "File watcher already started or not needed for URL:" << url.toString();
        return;
    }

    fmInfo() << "Starting file watcher for URL:" << url.toString();
    needStartWatcher = false;

    if (watcher) {
        fmDebug() << "Stopping existing watcher before restart";
        watcher->stopWatcher();
        watcher->disconnect(this);
    }

    // create watcher
    watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (watcher.isNull()) {
        fmWarning() << "Create watcher failed! url = " << url;
        return;
    }

    fmDebug() << "Connecting watcher signals for URL:" << url.toString();
    connect(watcher.data(), &AbstractFileWatcher::fileDeleted,
            this, &RootInfo::doFileDeleted);
    connect(watcher.data(), &AbstractFileWatcher::subfileCreated,
            this, &RootInfo::dofileCreated);
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
            this, &RootInfo::doFileUpdated);
    connect(watcher.data(), &AbstractFileWatcher::fileRename,
            this, &RootInfo::dofileMoved);

    watcher->restartWatcher();
    fmDebug() << "File watcher started successfully for URL:" << url.toString();
}

int RootInfo::clearTraversalThread(const QString &key, const bool isRefresh)
{
    if (!traversalThreads.contains(key)) {
        fmDebug() << "No traversal thread to clear for key:" << key;
        return traversalThreads.count();
    }

    fmDebug() << "Clearing traversal thread for key:" << key << "isRefresh:" << isRefresh;

    auto thread = traversalThreads.take(key);
    auto traversalThread = thread->traversalThread;
    if (traversalThread->isRunning()) {
        fmDebug() << "Emitting traversal finished signal for running thread";
        emit traversalFinished(key, false);
    }
    traversalThread->disconnect(this);
    if (traversalThread->isRunning()) {
        fmDebug() << "Moving running thread to discarded list";
        discardedThread.append(traversalThread);
        traversaling = false;
    }
    thread->traversalThread->stop();
    if (traversalThreads.isEmpty()) {
        fmDebug() << "All traversal threads cleared, enabling watcher restart";
        needStartWatcher = true;
    }

    this->isRefresh = isRefresh;
    fmDebug() << "Traversal threads remaining:" << traversalThreads.count();
    return traversalThreads.count();
}

void RootInfo::setFirstBatch(bool first)
{
    isFirstBatch = first;
}

void RootInfo::reset()
{
    fmInfo() << "Resetting RootInfo for URL:" << url.toString();

    disconnect();

    {
        QWriteLocker lk(&childrenLock);
        childrenUrlList.clear();
        sourceDataList.clear();
    }

    if (watcher) {
        fmDebug() << "Disconnecting and stopping file watcher";
        watcher->disconnect(this);
        watcher->stopWatcher();
    }

    traversaling = false;
    traversalFinish = false;

    cancelWatcherEvent = true;
    for (const auto &thread : traversalThreads) {
        thread->traversalThread->stop();
    }
    // wait old dir iterator thread
    for (const auto &thread : discardedThread) {
        thread->disconnect();
        thread->stop();
        thread->quit();
    }

    fmDebug() << "RootInfo reset completed";
}

bool RootInfo::canDelete() const
{
    for (auto &future : watcherEventFutures) {
        if (!future.isFinished()) {
            fmDebug() << "Cannot delete: watcher event future still running";
            return false;
        }
    }
    for (const auto &thread : traversalThreads) {
        if (!thread->traversalThread->isFinished()) {
            fmDebug() << "Cannot delete: traversal thread still running";
            return false;
        }
    }
    // wait old dir iterator thread
    for (const auto &thread : discardedThread) {
        if (!thread->isFinished()) {
            fmDebug() << "Cannot delete: discarded thread still running";
            return false;
        }
    }
    fmDebug() << "RootInfo can be safely deleted for URL:" << url.toString();
    return true;
}

QStringList RootInfo::getKeyWords() const
{
    return keyWords;
}

bool RootInfo::checkKeyOnly(const QString &key) const
{
    for (auto threadKey : traversalThreads.keys()) {
        if (threadKey != key)
            return false;
    }

    return true;
}

void RootInfo::doFileDeleted(const QUrl &url)
{
    fmDebug() << "File deleted event for URL:" << url.toString();
    enqueueEvent(QPair<QUrl, EventType>(url, kRmFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doThreadWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::dofileMoved(const QUrl &fromUrl, const QUrl &toUrl)
{
    fmInfo() << "File moved from:" << fromUrl.toString() << "to:" << toUrl.toString();
    Q_EMIT renameFileProcessStarted();
    doFileDeleted(fromUrl);

    AbstractFileInfoPointer info = InfoCacheController::instance().getCacheInfo(toUrl);
    if (info)
        info->refresh();

    dofileCreated(toUrl);
}

void RootInfo::dofileCreated(const QUrl &url)
{
    fmDebug() << "File created event for URL:" << url.toString();
    enqueueEvent(QPair<QUrl, EventType>(url, kAddFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doThreadWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::doFileUpdated(const QUrl &url)
{
    fmDebug() << "File updated event for URL:" << url.toString();
    enqueueEvent(QPair<QUrl, EventType>(url, kUpdateFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doThreadWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::doWatcherEvent()
{
    // 使用原子性的 test_and_set 操作
    bool expected = false;
    // 如果 processFileEventRuning 是 false，则将其设置为 true，并返回 true
    // 如果已经是 true，则什么都不做，并返回 false
    if (!processFileEventRuning.compare_exchange_strong(expected, true)) {
        fmDebug() << "Watcher event processing already running, skipping";
        return;   // 已经有其他线程在运行，直接返回
    }

    fmDebug() << "Starting watcher event processing for URL:" << url.toString();

    QElapsedTimer timer;
    timer.start();
    QList<QUrl> adds, updates, removes;
    qint64 oldtime = 0;
    int emptyLoopCount = 0;
    while (checkFileEventQueue() || timer.elapsed() < 200) {
        // 检查超时，重新设置起始时间
        if (timer.elapsed() - oldtime >= 200) {
            // 处理添加文件
            if (!adds.isEmpty()) {
                fmDebug() << "Processing" << adds.size() << "add events";
                addChildren(adds);
            }
            if (!updates.isEmpty()) {
                fmDebug() << "Processing" << updates.size() << "update events";
                updateChildren(updates);
            }
            if (!removes.isEmpty()) {
                fmDebug() << "Processing" << removes.size() << "remove events";
                removeChildren(removes);
            }

            adds.clear();
            updates.clear();
            removes.clear();

            oldtime = timer.elapsed();
        }

        if (cancelWatcherEvent) {
            fmDebug() << "Watcher event processing cancelled";
            return;
        }

        if (!checkFileEventQueue()) {
            if (emptyLoopCount >= 5)
                break;

            QThread::msleep(20);
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
                fmDebug() << "Root directory deleted, clearing all data for URL:" << url.toString();
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
    processFileEventRuning.store(false);   // 运行完毕，重置标志

    // 处理添加文件
    if (!removes.isEmpty())
        removeChildren(removes);
    if (!adds.isEmpty())
        addChildren(adds);
    if (!updates.isEmpty())
        updateChildren(updates);
}

void RootInfo::doThreadWatcherEvent()
{
    if (isDying.load() || processFileEventRuning)
        return;
    for (auto it = watcherEventFutures.begin(); it != watcherEventFutures.end();) {
        if (it->isFinished()) {
            it = watcherEventFutures.erase(it);
        } else {
            it++;
        }
    }

    watcherEventFutures << QtConcurrent::run([&]() {
        if (cancelWatcherEvent || isDying.load())
            return;
        doWatcherEvent();
    });
}

void RootInfo::handleTraversalResults(const QList<FileInfoPointer> children, const QString &travseToken)
{
    fmDebug() << "Handling traversal results for token:" << travseToken << "children count:" << children.size();

    QList<SortInfoPointer> sortInfos;
    QList<FileInfoPointer> infos;
    for (const auto &info : children) {
        auto sortInfo = addChild(info);
        if (!sortInfo)
            continue;
        sortInfos.append(sortInfo);

        infos.append(info);
    }

    if (sortInfos.length() > 0) {
        bool isFirst = isFirstBatch.exchange(false);   // Get and reset the flag
        fmDebug() << "Emitting iterator add files signal - sortInfos:" << sortInfos.size() << "isFirst:" << isFirst;
        Q_EMIT iteratorAddFiles(travseToken, sortInfos, infos, isFirst);
    }
}

void RootInfo::handleTraversalResultsUpdate(const QList<SortInfoPointer> children, const QString &travseToken)
{
    if (children.isEmpty()) {
        fmDebug() << "Empty traversal results update for token:" << travseToken;
        return;
    }

    QWriteLocker lk(&childrenLock);
    // 更新已存在的文件信息
    sourceDataList = children;

    bool isFirst = isFirstBatch.exchange(false);   // Get and reset the flag
    fmDebug() << "Emitting iterator update files signal - children:" << children.size() << "isFirst:" << isFirst;
    Q_EMIT iteratorUpdateFiles(travseToken, sourceDataList, isFirst);
}

void RootInfo::handleTraversalLocalResult(QList<SortInfoPointer> children,
                                          dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                                          Qt::SortOrder sortOrder, bool isMixDirAndFile, const QString &travseToken)
{
    originSortRole = sortRole;
    originSortOrder = sortOrder;
    originMixSort = isMixDirAndFile;

    if (children.isEmpty()) {
        fmDebug() << "Empty local traversal results for token:" << travseToken;
        return;
    }

    addChildren(children);

    bool isFirst = isFirstBatch.exchange(false);   // Get and reset the flag
    fmDebug() << "Emitting iterator local files signal - children:" << children.size() << "isFirst:" << isFirst;
    Q_EMIT iteratorLocalFiles(travseToken, children, originSortRole, originSortOrder, originMixSort, isFirst);
}

void RootInfo::handleTraversalFinish(const QString &travseToken)
{
    fmInfo() << "Traversal finished for token:" << travseToken << "URL:" << url.toString();

    traversaling = false;
    // Check if isFirstBatch is still true, which means no directory data was produced
    bool noDataProduced = isFirstBatch.load();
    // Reset isFirstBatch
    isFirstBatch.store(false);

    fmDebug() << "Emitting traversal finished signal - noDataProduced:" << noDataProduced;
    // Emit signal with additional parameter indicating if no data was produced
    emit traversalFinished(travseToken, noDataProduced);
    traversalFinish = true;
    if (isRefresh) {
        fmDebug() << "Refresh completed, resetting refresh flag";
        isRefresh = false;
    }
}

void RootInfo::handleTraversalSort(const QString &travseToken)
{
    fmDebug() << "Emitting traversal sort request for token:" << travseToken << "URL:" << url.toString();
    emit requestSort(travseToken, url);
}

void RootInfo::handleGetSourceData(const QString &currentToken)
{
    if (needStartWatcher)
        startWatcher();

    QList<SortInfoPointer> newDatas;
    bool isEmpty = false;
    {
        QReadLocker wlk(&childrenLock);
        newDatas = sourceDataList;
        isEmpty = sourceDataList.isEmpty();
    }

    fmDebug() << "Emitting source data signal - data count:" << newDatas.size() << "isEmpty:" << isEmpty << "token:" << currentToken;
    emit sourceDatas(currentToken, newDatas, originSortRole, originSortOrder, originMixSort, !traversaling);
    if (!traversaling)
        emit traversalFinished(currentToken, isEmpty);
}

void RootInfo::initConnection(const TraversalThreadManagerPointer &traversalThread)
{
    connect(traversalThread.data(), &TraversalDirThreadManager::updateChildrenManager,
            this, &RootInfo::handleTraversalResults, Qt::DirectConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::updateChildrenInfo,
            this, &RootInfo::handleTraversalResultsUpdate, Qt::DirectConnection);
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

    bool hasHiddenFile = false;
    for (auto url : urlList) {
        url.setPath(url.path());

        auto child = fileInfo(url);

        if (!child) {
            fmDebug() << "Failed to get file info for URL:" << url.toString();
            continue;
        }

        if (UniversalUtils::urlEquals(url, hiddenFileUrl))
            hasHiddenFile = true;

        auto sortInfo = addChild(child);
        if (sortInfo)
            newSortInfo.append(sortInfo);
    }

    if (newSortInfo.count() > 0) {
        originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
        fmDebug() << "Emitting watcher add files signal - count:" << newSortInfo.count();
        emit watcherAddFiles(newSortInfo);
    }

    if (hasHiddenFile) {
        fmDebug() << "Emitting watcher update hide file signal for:" << hiddenFileUrl.toString();
        Q_EMIT watcherUpdateHideFile(hiddenFileUrl);
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
    if (!child) {
        fmDebug() << "Cannot add child: null file info";
        return nullptr;
    }

    QUrl childUrl = child->urlOf(dfmbase::UrlInfoType::kUrl);
    childUrl.setPath(childUrl.path());

    SortInfoPointer sort = sortFileInfo(child);
    if (!sort) {
        fmDebug() << "Failed to create sort info for child:" << childUrl.toString();
        return nullptr;
    }

    {
        QWriteLocker lk(&childrenLock);
        if (childrenUrlList.contains(childUrl)) {
            fmDebug() << "Replacing existing child:" << childUrl.toString();
            sourceDataList.replace(childrenUrlList.indexOf(childUrl), sort);
            return sort;
        }
        childrenUrlList.append(childUrl);
        sourceDataList.append(sort);
        fmDebug() << "Added new child:" << childUrl.toString() << "total children:" << childrenUrlList.size();
    }

    return sort;
}

SortInfoPointer RootInfo::sortFileInfo(const FileInfoPointer &info)
{
    if (!info) {
        fmDebug() << "Cannot create sort info: null file info";
        return nullptr;
    }

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
    sortInfo->setLastReadTime(info->timeOf(TimeInfoType::kLastRead).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setLastModifiedTime(info->timeOf(TimeInfoType::kLastModified).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setCreateTime(info->timeOf(TimeInfoType::kCreateTime).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setInfoCompleted(true);

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

    if (removeUrls.contains(hiddenFileUrl))
        Q_EMIT watcherUpdateHideFile(hiddenFileUrl);
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
    if (info.isNull()) {
        fmWarning() << "Failed to get file info for update:" << url.toString();
        return nullptr;
    }

    auto realUrl = info->urlOf(UrlInfoType::kUrl);

    {
        QReadLocker lk(&childrenLock);
        if (!childrenUrlList.contains(realUrl)) {
            fmDebug() << "Child not found in list for update:" << realUrl.toString();
            return nullptr;
        }
    }

    sort = sortFileInfo(info);
    if (sort.isNull()) {
        fmWarning() << "Failed to create sort info for update:" << url.toString();
        return nullptr;
    }

    {
        QWriteLocker lk(&childrenLock);
        sourceDataList.replace(childrenUrlList.indexOf(realUrl), sort);
    }
    // NOTE: GlobalEventType::kHideFiles event is watched in fileview, but this can be used to notify update view
    // when the file is modified in other way.
    if (UniversalUtils::urlEquals(hiddenFileUrl, url)) {
        fmDebug() << "Emitting watcher update hide file signal for:" << url.toString();
        Q_EMIT watcherUpdateHideFile(url);
    }

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
    if (updates.isEmpty()) {
        fmDebug() << "No valid updates generated";
        return;
    }
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

    if (!watcher) {
        fmDebug() << "No watcher available for fallback file info creation";
        return nullptr;
    }

    const QUrl &parentUrl = QUrl::fromPercentEncoding(watcher->url().toString().toUtf8());
    auto path = url.path();
    if (path.isEmpty() || path == QDir::separator() || url.fileName().isEmpty()) {
        fmDebug() << "Invalid path for fallback file info creation:" << path;
        return info;
    }

    auto pathParent = path.endsWith(QDir::separator()) ? path.left(path.length() - 1) : path;
    auto parentPath = parentUrl.path().endsWith(QDir::separator())
            ? parentUrl.path().left(parentUrl.path().length() - 1)
            : parentUrl.path();
    pathParent = pathParent.left(pathParent.lastIndexOf(QDir::separator()));
    if (!parentPath.endsWith(pathParent.mid(1))) {
        fmDebug() << "Path mismatch for fallback file info creation";
        return info;
    }

    auto currentUrl = parentUrl;
    currentUrl.setPath(currentUrl.path(QUrl::PrettyDecoded) + QDir::separator() + url.fileName());
    info = InfoFactory::create<FileInfo>(currentUrl);
    return info;
}
