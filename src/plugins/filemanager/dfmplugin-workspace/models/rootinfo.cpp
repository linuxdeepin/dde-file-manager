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

static constexpr int kWatcherEventDelayMs = 100;

FileWatcherWorker::FileWatcherWorker(RootInfoWorker *root, QObject *parent)
    : QObject(parent), rootptr(root)
{
}

FileWatcherWorker::~FileWatcherWorker()
{
}

void FileWatcherWorker::doFileDeleted(const QUrl &fileUrl)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    fmDebug() << "FileWatcherWorker::doFileDeleted: url=" << fileUrl << "stopped=" << rootptr->stopped;
    if (rootptr->stopped || !isSubFile(fileUrl))
        return;

    if (UniversalUtils::urlEquals(fileUrl, rootptr->url)) {
        fmWarning() << "Root directory deleted:" << fileUrl;
        emit InfoCacheController::instance().removeCacheFileInfo({ fileUrl });
        WatcherCache::instance().removeCacheWatcherByParent(fileUrl);

        emit rootptr->requestCloseTab(fileUrl);
        emit rootptr->requestClearRoot(fileUrl);
        rootptr->childrenUrlList.clear();
        rootptr->sourceDataList.clear();
        return;
    }

    adds.remove(fileUrl);
    updates.remove(fileUrl);
    if (!removes.contains(fileUrl))
        removes.insert(fileUrl);
    doCheckAndStartTimer();
}

void FileWatcherWorker::doFileMoved(const QUrl &fromUrl, const QUrl &toUrl)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    fmInfo() << "FileWatcherWorker::dofileMoved: from=" << fromUrl << "to=" << toUrl;
    if (rootptr->stopped)
        return;
    emit rootptr->renameFileProcessStarted();
    doFileDeleted(fromUrl);
    auto info = InfoCacheController::instance().getCacheInfo(toUrl);
    if (info)
        info->updateAttributes();

    doFileCreated(toUrl);
}

void FileWatcherWorker::doFileCreated(const QUrl &fileUrl)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    fmDebug() << "FileWatcherWorker::doFileCreated: url=" << fileUrl << "stopped=" << rootptr->stopped;
    if (rootptr->stopped || !isSubFile(fileUrl))
        return;

    updates.remove(fileUrl);
    removes.remove(fileUrl);
    if (!adds.contains(fileUrl))
        adds.insert(fileUrl);

    doCheckAndStartTimer();
}

void FileWatcherWorker::doFileUpdated(const QUrl &fileUrl)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    fmDebug() << "FileWatcherWorker::doFileUpdated: url=" << fileUrl << "stopped=" << rootptr->stopped;
    if (rootptr->stopped || !isSubFile(fileUrl) || UniversalUtils::urlEquals(fileUrl, rootptr->url))
        return;

    if (adds.contains(fileUrl) || removes.contains(fileUrl) || updates.contains(fileUrl))
        return;

    bool fileShowNow = rootptr->childrenUrlList.contains(fileUrl);
    if (fileShowNow) {
        updates.insert(fileUrl);
    } else if (dfmio::DFile(fileUrl).exists()) {
        adds.insert(fileUrl);
    }
    doCheckAndStartTimer();
}

void FileWatcherWorker::doWatcherEvent()
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    delayTimeStart = false;
    doWatcherSubEvent();
}

void FileWatcherWorker::doWatcherSubEvent()
{
    if (rootptr->stopped)
        return;
    if (!removes.isEmpty()) {
        rootptr->removeChildren(removes);
        removes.clear();
    }
    if (!adds.isEmpty()) {
        rootptr->addChildren(adds);
        adds.clear();
    }
    if (!updates.isEmpty()) {
        rootptr->updateChildren(updates);
        updates.clear();
    }
}

void FileWatcherWorker::doCheckAndStartTimer()
{
    if (delayTimeStart)
        return;
    delayTimeStart = true;
    QTimer::singleShot(kWatcherEventDelayMs, this, [this]{ doWatcherEvent(); });
}

bool FileWatcherWorker::isSubFile(const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile())
        return true;

    auto url = fileUrl;
    auto parentUrl = rootptr->url;
    parentUrl.setUserInfo(QString());
    parentUrl.setQuery(QString());
    url.setUserInfo(QString());
    url.setQuery(QString());

    auto parentPath = parentUrl.path();
    auto filePath = url.path();
    if (parentPath.endsWith(QDir::separator()))
        parentPath.chop(1);
    if (!filePath.startsWith(parentPath))
        return false;

    return true;
}

FileIteratorWorker::FileIteratorWorker(RootInfoWorker *root, QObject *parent)
    : QObject(parent), rootptr(root)
{
}

FileIteratorWorker::~FileIteratorWorker()
{
}

void FileIteratorWorker::handleTraversalResults(const QList<FileInfoPointer> &children, const QString &travseToken)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    if (rootptr->stopped)
        return;

    QList<SortInfoPointer> sortInfos;
    QList<FileInfoPointer> infos;
    std::for_each(children.begin(), children.end(), [this, &sortInfos, &infos](const FileInfoPointer &info){
        if (rootptr->stopped)
            return;
        auto sortInfo = rootptr->addChild(info);
        if (!sortInfo)
            return;
        sortInfos.append(sortInfo);

        infos.append(info);
    });

    fmDebug() << "FileIteratorWorker::handleTraversalResults: sortInfos count=" << sortInfos.size();
    if (sortInfos.length() > 0)
        Q_EMIT rootptr->iteratorAddFiles(travseToken, sortInfos, infos);
}

void FileIteratorWorker::handleTraversalResultsUpdate(const QList<SortInfoPointer> &children, const QString &travseToken, bool increment)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    if (children.isEmpty() || rootptr->stopped)
        return;

    if (!increment) {
        rootptr->replaceChildren(children);
    } else {
        for (const auto &file : children) {
            if (!file)
                continue;
            rootptr->childrenUrlList.append(file->fileUrl());
            rootptr->sourceDataList.append(file);
        }
    }

    emit rootptr->iteratorUpdateFiles(travseToken, rootptr->sourceDataList, false);
}

void FileIteratorWorker::handleTraversalLocalResult(QList<SortInfoPointer> children, dfmio::DEnumerator::SortRoleCompareFlag sortRole, Qt::SortOrder sortOrder, bool isMixDirAndFile, const QString &travseToken)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    if (rootptr->stopped)
        return;
    rootptr->originSortRole = sortRole;
    rootptr->originSortOrder = sortOrder;
    rootptr->originMixSort = isMixDirAndFile;

    rootptr->replaceChildren(children);

    Q_EMIT rootptr->iteratorLocalFiles(travseToken, rootptr->sourceDataList, rootptr->originSortRole, rootptr->originSortOrder, rootptr->originMixSort, true);
}

void FileIteratorWorker::handleTraversalFinish(const QString &travseToken)
{
    if (rootptr->stopped)
        return;
    rootptr->itStatus = RootInfoWorker::IteratorStatus::kFinished;
    emit rootptr->traversalFinished(travseToken);
}

void FileIteratorWorker::handleTraversalSort(const QString &travseToken)
{
    if (rootptr->stopped)
        return;
    emit rootptr->requestSort(travseToken, rootptr->url);
}

RootInfoWorker::RootInfoWorker(const QUrl &url, QObject *parent)
    : QObject(parent), url(url)
{
    hiddenFileUrl.setScheme(url.scheme());
    hiddenFileUrl.setPath(DFMIO::DFMUtils::buildFilePath(url.path().toStdString().c_str(), ".hidden", nullptr));
    it.reset(new FileIteratorWorker(this));
    watch.reset(new FileWatcherWorker(this));
}

RootInfoWorker::~RootInfoWorker()
{
}

void RootInfoWorker::stop()
{
    stopped = true;
}

QSharedPointer<FileIteratorWorker> RootInfoWorker::iteratorWorker() const
{
    return it;
}

QSharedPointer<FileWatcherWorker> RootInfoWorker::watcherWorker() const
{
    return watch;
}

void RootInfoWorker::addChildren(const QSet<QUrl> &urlList)
{
    if (stopped)
        return;
    fmInfo() << "RootInfoWorker::addChildren(QSet): url count=" << urlList.size() << "stopped=" << stopped;
    QList<SortInfoPointer> newSortInfo;

    bool isContainHidd = false;
    std::for_each(urlList.begin(), urlList.end(), [this, &isContainHidd, &newSortInfo](const QUrl &url) {
        if (stopped)
            return;

        auto child = fileInfo(url);

        if (!child)
            return;

        if (UniversalUtils::urlEquals(url, hiddenFileUrl))
            isContainHidd = true;

        auto sortInfo = addChild(child);
        if (sortInfo)
            newSortInfo.append(sortInfo);
    });

    fmDebug() << "RootInfoWorker::addChildren(QSet): newSortInfo count=" << newSortInfo.size();
    if (newSortInfo.count() > 0) {
        originSortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
        emit watcherAddFiles(newSortInfo);
    }

    if (isContainHidd)
        Q_EMIT watcherUpdateHideFile(hiddenFileUrl);
}

void RootInfoWorker::replaceChildren(const QList<SortInfoPointer> &children)
{
    if (stopped)
        return;

    childrenUrlList.clear();
    sourceDataList = children;

    std::for_each(children.begin(), children.end(), [this](const SortInfoPointer &sortInfo) {
        if (stopped || !sortInfo)
            return;
        childrenUrlList.append(sortInfo->fileUrl());
    });
}

SortInfoPointer RootInfoWorker::addChild(const FileInfoPointer &child)
{
    if (stopped || !child)
        return nullptr;

    auto childUrl = child->urlOf(UrlInfoType::kUrl);
    if (childrenUrlList.contains(childUrl))
        return nullptr;

    auto sort = sortFileInfo(child);
    if (!sort)
        return nullptr;

    childrenUrlList.append(childUrl);
    sourceDataList.append(sort);

    return sort;
}

SortInfoPointer RootInfoWorker::sortFileInfo(const FileInfoPointer &info)
{
    if (!info || stopped)
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

    sortInfo->setLastReadTime(info->timeOf(TimeInfoType::kLastRead).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setLastModifiedTime(info->timeOf(TimeInfoType::kLastModified).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setCreateTime(info->timeOf(TimeInfoType::kCreateTime).value<QDateTime>().toSecsSinceEpoch());
    sortInfo->setInfoCompleted(true);

    return sortInfo;
}

void RootInfoWorker::removeChildren(const QSet<QUrl> &urlList)
{
    if (stopped)
        return;
    QList<SortInfoPointer> removeChildren {};
    QList<QUrl> removeUrls;

    std::for_each(urlList.begin(), urlList.end(), [this, &removeUrls, &removeChildren](const QUrl &fileUrl){
        if (stopped)
            return;

        QUrl realUrl = fileUrl;
        int childIndex = childrenUrlList.indexOf(fileUrl);
        FileInfoPointer childInfo;

        if (childIndex < 0) {
            childInfo = fileInfo(fileUrl);
            if (!childInfo)
                return;
            realUrl = childInfo->urlOf(UrlInfoType::kUrl);
            childIndex = childrenUrlList.indexOf(realUrl);
        }

        bool isDir;
        if (childIndex >= 0 && childIndex < sourceDataList.length()) {
            isDir = sourceDataList.at(childIndex)->isDir();
        } else {
            if (!childInfo) {
                childInfo = fileInfo(fileUrl);
                if (!childInfo)
                    return;
            }
            isDir = childInfo->isDir();
        }

        if (isDir) {
            WatcherCache::instance().removeCacheWatcherByParent(realUrl);
            emit requestCloseTab(realUrl);
        }

        removeUrls.append(realUrl);
        if (childIndex < 0 || childIndex >= childrenUrlList.length()) {
            if (childInfo)
                removeChildren.append(sortFileInfo(childInfo));
            return;
        }
        childrenUrlList.removeAt(childIndex);
        removeChildren.append(sourceDataList.takeAt(childIndex));
    });

    if (removeUrls.count() > 0)
        emit InfoCacheController::instance().removeCacheFileInfo(removeUrls);

    if (removeChildren.count() > 0)
        emit watcherRemoveFiles(removeChildren);

    if (removeUrls.contains(hiddenFileUrl))
        Q_EMIT watcherUpdateHideFile(hiddenFileUrl);
}

FileInfoPointer RootInfoWorker::fileInfo(const QUrl &url)
{
    if (stopped)
        return nullptr;
    FileInfoPointer info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!info.isNull())
        return info;

    const QUrl &parentUrl = QUrl::fromPercentEncoding(this->url.toString().toUtf8());
    auto path = url.path();
    if (path.isEmpty() || path == QDir::separator() || url.fileName().isEmpty())
        return info;

    auto pathParent = path.endsWith(QDir::separator()) ? path.left(path.length() - 1) : path;
    pathParent = pathParent.left(pathParent.lastIndexOf(QDir::separator()));

    auto parentPath = parentUrl.path();
    if (parentPath.endsWith(QDir::separator()))
        parentPath.chop(1);

    if (QDir::cleanPath(parentPath) != QDir::cleanPath(pathParent))
        return info;

    auto currentUrl = parentUrl;
    currentUrl.setPath(currentUrl.path(QUrl::PrettyDecoded) + QDir::separator() + url.fileName());
    info = InfoFactory::create<FileInfo>(currentUrl);
    return info;
}

SortInfoPointer RootInfoWorker::updateChild(const QUrl &url)
{
    if (stopped)
        return nullptr;
    SortInfoPointer sort { nullptr };

    auto info = fileInfo(url);
    if (info.isNull())
        return nullptr;

    auto realUrl = info->urlOf(UrlInfoType::kUrl);
    if (!childrenUrlList.contains(realUrl))
        return nullptr;
    sort = sortFileInfo(info);
    if (sort.isNull())
        return nullptr;
    sourceDataList.replace(childrenUrlList.indexOf(realUrl), sort);

    if (UniversalUtils::urlEquals(hiddenFileUrl, url))
        Q_EMIT watcherUpdateHideFile(url);

    return sort;
}

void RootInfoWorker::updateChildren(const QSet<QUrl> &urls)
{
    if (stopped)
        return;
    QList<SortInfoPointer> updates;
    std::for_each(urls.begin(), urls.end(), [this, &updates](const QUrl url) {
        if (stopped)
            return;
        auto sort = updateChild(url);
        if (sort)
            updates.append(sort);
    });
    if (updates.isEmpty())
        return;
    emit watcherUpdateFiles(updates);
}

void RootInfoWorker::onResetData()
{
    childrenUrlList.clear();
    sourceDataList.clear();
}

void RootInfoWorker::onSetIteratorStatus(const RootInfoWorker::IteratorStatus &status)
{
    itStatus = status;
}

RootInfo::RootInfo(const QUrl &u, QObject *parent)
    : QObject(parent), url(u)
{
    fmInfo() << "RootInfo created for url:" << u;
    rootWorker.reset(new RootInfoWorker(url));

    initConnection();
    rootWorker->moveToThread(&rootThread);
    rootWorker->watcherWorker()->moveToThread(&rootThread);
    rootWorker->iteratorWorker()->moveToThread(&rootThread);

    keyWords = KeywordExtractorManager::instance().extractor().extractFromUrl(url);

    connect(qApp, &QApplication::aboutToQuit, this, [this]{
        this->clearAllThread();
    });
    rootThread.start();
}

RootInfo::~RootInfo()
{
    fmInfo() << "RootInfo destroyed for url:" << url;
    clearAllThread();
}

void RootInfo::initThreadOfFileData(const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order, bool isMixFileAndFolder)
{
    for (auto it = discardedThread.begin(); it != discardedThread.end(); ) {
        if (!(*it)->isRunning()) {
            it = discardedThread.erase(it);
        } else {
            it++;
        }
    }
    QSharedPointer<DirIteratorThread> traversalThread = traversalThreads.value(key);
    if (!traversalThread.isNull()) {
        traversalThread->traversalThread->disconnect();
    }

    traversalThread.reset(new DirIteratorThread);
    traversalThread->traversalThread.reset(
            new TraversalDirThreadManager(url, QStringList(),
                                          QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
                                          QDirIterator::FollowSymlinks));
    traversalThread->traversalThread->setSortAgruments(order, role, isMixFileAndFolder);
    traversalThread->traversalThread->setTraversalToken(key);
    initIteratorConnection(traversalThread->traversalThread);
    traversalThreads.insert(key, traversalThread);
}

void RootInfo::startIteratorWork(const QString &key)
{
    if (!traversalThreads.contains(key)) {
        fmWarning() << "startIteratorWork: key not found" << key;
        return;
    }

    fmInfo() << "startIteratorWork: key=" << key << "url=" << url;
    emit resetData();
    emit iteratorStatus(RootInfoWorker::IteratorStatus::kRunning);
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

    watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (watcher.isNull()) {
        fmWarning() << "Create watcher failed! url = " << url;
        return;
    }

    connect(watcher.data(), &AbstractFileWatcher::fileDeleted,
            rootWorker->watcherWorker().data(), &FileWatcherWorker::doFileDeleted, Qt::QueuedConnection);
    connect(watcher.data(), &AbstractFileWatcher::subfileCreated,
            rootWorker->watcherWorker().data(), &FileWatcherWorker::doFileCreated, Qt::QueuedConnection);
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
            rootWorker->watcherWorker().data(), &FileWatcherWorker::doFileUpdated, Qt::QueuedConnection);
    connect(watcher.data(), &AbstractFileWatcher::fileRename,
            rootWorker->watcherWorker().data(), &FileWatcherWorker::doFileMoved, Qt::QueuedConnection);

    watcher->restartWatcher();
}

int RootInfo::clearTraversalThread(const QString &key, const bool isRefresh)
{
    if (!traversalThreads.contains(key))
        return traversalThreads.count();

    auto thread = traversalThreads.take(key);
    auto traversalThread = thread->traversalThread;
    traversalThread->disconnect(this);
    if (traversalThread->isRunning()) {
        discardedThread.append(traversalThread);
    }
    thread->traversalThread->stop();
    if (traversalThreads.isEmpty())
        needStartWatcher = true;

    this->isRefresh = isRefresh;
    return traversalThreads.count();
}

void RootInfo::setFirstBatch(bool first)
{
    isFirstBatch.store(first);
}

void RootInfo::reset()
{
    fmInfo() << "RootInfo::reset: url=" << url;

    emit resetData();

    if (watcher) {
        watcher->stopWatcher();
    }
    needStartWatcher = true;

    emit iteratorStatus(RootInfoWorker::IteratorStatus::kNone);

    for (const auto &thread : traversalThreads) {
        thread->traversalThread->stop();
    }
    for (const auto &thread : discardedThread) {
        thread->disconnect();
        thread->stop();
        thread->quit();
    }

    fmDebug() << "RootInfo reset completed";
}

bool RootInfo::canDelete() const
{
    for (const auto &thread : traversalThreads) {
        if (!thread->traversalThread->isFinished())
            return false;
    }
    for (const auto &thread : discardedThread) {
        if (!thread->isFinished())
            return false;
    }
    return true;
}

bool RootInfo::checkKeyOnly(const QString &key) const
{
    for (auto threadKey : traversalThreads.keys()) {
        if (threadKey != key)
            return false;
    }

    return true;
}

QStringList RootInfo::getKeyWords() const
{
    return keyWords;
}

void RootInfo::addConnectToken(const QString &token)
{
    if (connectedTokens.contains(token))
        return;
    connectedTokens << token;
}

QStringList RootInfo::connectTokens() const
{
    return connectedTokens;
}

void RootInfo::handleTraversalFinish(const QString &travseToken)
{
    fmInfo() << "RootInfo::handleTraversalFinish:" << travseToken << "url:" << url;

    bool noDataProduced = isFirstBatch.load();
    isFirstBatch.store(false);

    fmDebug() << "Emitting traversal finished signal - noDataProduced:" << noDataProduced;
    emit traversalFinished(travseToken, noDataProduced);

    if (isRefresh) {
        fmDebug() << "RootInfo::handleTraversalFinish: refresh completed";
        isRefresh = false;
    }
}

void RootInfo::initIteratorConnection(const TraversalThreadManagerPointer &traversalThread)
{
    connect(traversalThread.data(), &TraversalDirThreadManager::updateChildrenManager,
            rootWorker->iteratorWorker().data(), &FileIteratorWorker::handleTraversalResults, Qt::QueuedConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::updateLocalChildren,
            rootWorker->iteratorWorker().data(), &FileIteratorWorker::handleTraversalLocalResult, Qt::QueuedConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::traversalRequestSort,
            rootWorker->iteratorWorker().data(), &FileIteratorWorker::handleTraversalSort, Qt::QueuedConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::updateChildrenInfo,
            rootWorker->iteratorWorker().data(), &FileIteratorWorker::handleTraversalResultsUpdate, Qt::QueuedConnection);
    connect(traversalThread.data(), &TraversalDirThreadManager::traversalFinished,
            rootWorker->iteratorWorker().data(), &FileIteratorWorker::handleTraversalFinish, Qt::QueuedConnection);

    connect(traversalThread.data(), &TraversalDirThreadManager::iteratorInitFinished,
            this, &RootInfo::startWatcher, Qt::QueuedConnection);
}

void RootInfo::initConnection()
{
    connect(rootWorker.data(), &RootInfoWorker::iteratorLocalFiles, this,
            [this](const QString &key,
                   const QList<SortInfoPointer> children,
                   const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                   const Qt::SortOrder sortOrder,
                   const bool isMixDirAndFile, const bool isFirst) {
                Q_UNUSED(isFirst)
                bool firstBatch = children.isEmpty() ? false : isFirstBatch.exchange(false);
                emit iteratorLocalFiles(key, children, sortRole, sortOrder, isMixDirAndFile, firstBatch);
            }, Qt::QueuedConnection);

    connect(rootWorker.data(), &RootInfoWorker::iteratorUpdateFiles, this,
            [this](const QString &key, const QList<SortInfoPointer> children, const bool isFirst) {
                Q_UNUSED(isFirst)
                bool firstBatch = children.isEmpty() ? false : isFirstBatch.exchange(false);
                emit iteratorUpdateFiles(key, children, firstBatch);
            }, Qt::QueuedConnection);

    connect(rootWorker.data(), &RootInfoWorker::iteratorAddFiles, this,
            [this](const QString &key, const QList<SortInfoPointer> sortInfos, const QList<FileInfoPointer> infos) {
                bool firstBatch = sortInfos.isEmpty() ? false : isFirstBatch.exchange(false);
                emit iteratorAddFiles(key, sortInfos, infos, firstBatch);
            }, Qt::QueuedConnection);

    connect(rootWorker.data(), &RootInfoWorker::requestSort, this, &RootInfo::requestSort, Qt::QueuedConnection);
    connect(rootWorker.data(), &RootInfoWorker::traversalFinished, this, &RootInfo::handleTraversalFinish, Qt::QueuedConnection);

    connect(rootWorker.data(), &RootInfoWorker::watcherAddFiles, this, &RootInfo::watcherAddFiles, Qt::QueuedConnection);
    connect(rootWorker.data(), &RootInfoWorker::watcherRemoveFiles, this, &RootInfo::watcherRemoveFiles, Qt::QueuedConnection);
    connect(rootWorker.data(), &RootInfoWorker::watcherUpdateHideFile, this, &RootInfo::watcherUpdateHideFile, Qt::QueuedConnection);
    connect(rootWorker.data(), &RootInfoWorker::watcherUpdateFile, this, &RootInfo::watcherUpdateFile, Qt::QueuedConnection);
    connect(rootWorker.data(), &RootInfoWorker::watcherUpdateFiles, this, &RootInfo::watcherUpdateFiles, Qt::QueuedConnection);
    connect(rootWorker.data(), &RootInfoWorker::renameFileProcessStarted, this, &RootInfo::renameFileProcessStarted, Qt::QueuedConnection);

    connect(rootWorker.data(), &RootInfoWorker::requestCloseTab, this, &RootInfo::requestCloseTab, Qt::QueuedConnection);
    connect(rootWorker.data(), &RootInfoWorker::requestClearRoot, this, &RootInfo::requestClearRoot, Qt::QueuedConnection);

    connect(this, &RootInfo::resetData, rootWorker.data(), &RootInfoWorker::onResetData, Qt::QueuedConnection);
    connect(this, &RootInfo::iteratorStatus, rootWorker.data(), &RootInfoWorker::onSetIteratorStatus, Qt::QueuedConnection);
}

void RootInfo::clearAllThread()
{
    disconnect();
    if (watcher)
        watcher->stopWatcher();

    for (const auto &thread : traversalThreads) {
        thread->traversalThread->stop();
        thread->traversalThread->wait();
    }
    for (const auto &thread : discardedThread) {
        thread->disconnect();
        thread->stop();
        thread->quit();
        thread->wait();
    }

    if (rootWorker)
        rootWorker->stop();

    rootThread.quit();
    rootThread.wait();
}
