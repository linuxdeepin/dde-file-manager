// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traversaldirthreadmanager.h"
#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/utils/fileutils.h>

#include <QElapsedTimer>
#include <QDebug>

typedef QList<QSharedPointer<DFMBASE_NAMESPACE::SortFileInfo>> &SortInfoList;

using namespace dfmbase;
using namespace dfmplugin_workspace;
USING_IO_NAMESPACE

TraversalDirThreadManager::TraversalDirThreadManager(const QUrl &url,
                                                     const QStringList &nameFilters,
                                                     QDir::Filters filters,
                                                     QDirIterator::IteratorFlags flags,
                                                     QObject *parent)
    : TraversalDirThread(url, nameFilters, filters, flags, parent)
{
    fmDebug() << "TraversalDirThreadManager created for URL:" << url.toString()
              << "name filters count:" << nameFilters.size() << "filters:" << filters;

    qRegisterMetaType<QList<FileInfoPointer>>();
    qRegisterMetaType<FileInfoPointer>();
    qRegisterMetaType<QList<SortInfoPointer>>();
    qRegisterMetaType<SortInfoPointer>();
    traversalToken = QString::number(quintptr(this), 16);

    fmDebug() << "TraversalDirThreadManager initialization completed, token:" << traversalToken;
}

TraversalDirThreadManager::~TraversalDirThreadManager()
{
    fmDebug() << "TraversalDirThreadManager destructor started, token:" << traversalToken;

    quit();
    wait();
    if (future) {
        fmDebug() << "Cleaning up async iterator future";
        future->deleteLater();
        future = nullptr;
    }

    fmDebug() << "TraversalDirThreadManager destructor completed, token:" << traversalToken;
}

void TraversalDirThreadManager::setSortAgruments(const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile)
{
    fmDebug() << "Setting sort arguments - order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending")
              << "role:" << sortRole << "mix dir and file:" << isMixDirAndFile;

    sortOrder = order;
    this->isMixDirAndFile = isMixDirAndFile;
    switch (sortRole) {
    case Global::ItemRoles::kItemFileDisplayNameRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileName;
        fmDebug() << "Sort role set to FileName";
        break;
    case Global::ItemRoles::kItemFileSizeRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileSize;
        fmDebug() << "Sort role set to FileSize";
        break;
    case Global::ItemRoles::kItemFileLastReadRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastRead;
        fmDebug() << "Sort role set to FileLastRead";
        break;
    case Global::ItemRoles::kItemFileLastModifiedRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastModified;
        fmDebug() << "Sort role set to FileLastModified";
        break;
    default:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
        fmDebug() << "Sort role set to Default";
    }
}

void TraversalDirThreadManager::setTraversalToken(const QString &token)
{
    fmDebug() << "Setting traversal token from" << traversalToken << "to" << token;
    traversalToken = token;
}

void TraversalDirThreadManager::start()
{
    fmInfo() << "Starting TraversalDirThreadManager for URL:" << dirUrl.toString() << "token:" << traversalToken;

    running = true;
    if (this->sortRole != dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault
        && dirIterator->oneByOne()) {
        fmDebug() << "Setting QueryAttributes for sorted one-by-one iteration";
        dirIterator->setProperty("QueryAttributes", "standard::name,standard::type,standard::is-file,standard::is-dir,"
                                                    "standard::size,standard::is-symlink,standard::symlink-target,access::*,time::*");
    }

    auto local = dirIterator.dynamicCast<LocalDirIterator>();
    if (local && local->oneByOne()) {
        fmDebug() << "Using async iterator for local directory";
        future = local->asyncIterator();
        if (future) {
            connect(future, &DEnumeratorFuture::asyncIteratorOver, this, &TraversalDirThreadManager::onAsyncIteratorOver);
            future->startAsyncIterator();
            fmDebug() << "Async iterator started successfully";
            return;
        } else {
            fmWarning() << "Failed to create async iterator, falling back to sync mode";
        }
    }

    fmDebug() << "Starting synchronous traversal thread";
    TraversalDirThread::start();
}

bool TraversalDirThreadManager::isRunning() const
{
    return running;
}

void TraversalDirThreadManager::onAsyncIteratorOver()
{
    fmDebug() << "Async iterator completed, starting main traversal thread, token:" << traversalToken;
    Q_EMIT iteratorInitFinished();
    TraversalDirThread::start();
}

void TraversalDirThreadManager::run()
{
    if (dirIterator.isNull()) {
        fmWarning() << "Directory iterator is null, cannot start traversal, token:" << traversalToken;
        emit traversalFinished(traversalToken);
        running = false;
        return;
    }

    QElapsedTimer timer;
    timer.start();
    fmInfo() << "dir query start, url: " << dirUrl;

    int count = 0;
    if (!dirIterator->oneByOne()) {
        const QList<SortInfoPointer> &fileList = iteratorAll();
        count = fileList.count();
        fmInfo() << "local dir query end, file count: " << count << " url: " << dirUrl << " elapsed: " << timer.elapsed();
    } else {
        count = iteratorOneByOne(timer);
        fmInfo() << "dir query end, file count: " << count << " url: " << dirUrl << " elapsed: " << timer.elapsed();
    }
    running = false;
}

int TraversalDirThreadManager::iteratorOneByOne(const QElapsedTimer &timere)
{
    dirIterator->cacheBlockIOAttribute();
    fmInfo() << "cacheBlockIOAttribute finished, url: " << dirUrl << " elapsed: " << timere.elapsed();
    if (stopFlag) {
        fmDebug() << "Stop flag detected during cache block IO, aborting traversal";
        emit traversalFinished(traversalToken);
        return 0;
    }

    if (!dirIterator->initIterator()) {
        fmWarning() << "dir iterator init failed !! url : " << dirUrl;
        emit traversalFinished(traversalToken);
        return 0;
    }

    if (!future)
        Q_EMIT iteratorInitFinished();

    timer.restart();

    QList<FileInfoPointer> childrenList;   // 当前遍历出来的所有文件
    QSet<QUrl> urls;
    int filecount = 0;
    bool noCache = dirIterator->property("FileInfoNoCache").toBool();
    while (dirIterator->hasNext()) {
        if (stopFlag) {
            fmDebug() << "Stop flag detected during iteration, processed" << filecount << "files";
            break;
        }

        // 调用一次fileinfo进行文件缓存
        const auto &fileUrl = dirIterator->next();
        if (!fileUrl.isValid())
            continue;
        if (urls.contains(fileUrl))
            continue;
        urls.insert(fileUrl);

        auto fileInfo = dirIterator->fileInfo();
        if (fileUrl.isValid() && !fileInfo) {
            fileInfo = InfoFactory::create<FileInfo>(fileUrl,
                                                     noCache ? Global::CreateFileInfoType::kCreateFileInfoAutoNoCache
                                                             : Global::CreateFileInfoType::kCreateFileInfoAuto);
        } else if (!fileInfo.isNull() && !noCache) {
            InfoFactory::cacheFileInfo(fileInfo);
        }

        if (!fileInfo)
            continue;

        childrenList.append(fileInfo);
        filecount++;

        if (timer.elapsed() > timeCeiling || childrenList.count() > countCeiling) {
            emit updateChildrenManager(childrenList, traversalToken);
            timer.restart();
            childrenList.clear();
        }
    }

    if (childrenList.length() > 0)
        emit updateChildrenManager(childrenList, traversalToken);

    if (!dirIterator->property(IteratorProperty::kKeepOrder).toBool()) {
        fmDebug() << "Requesting sort for unordered results";
        emit traversalRequestSort(traversalToken);
    }

    emit traversalFinished(traversalToken);

    return filecount;
}

QList<SortInfoPointer> TraversalDirThreadManager::iteratorAll()
{
    fmDebug() << "Starting batch mode iteration for URL:" << dirUrl.toString();

    QVariantMap args;
    args.insert("sortRole",
                QVariant::fromValue(sortRole));
    args.insert("mixFileAndDir", isMixDirAndFile);
    args.insert("sortOrder", sortOrder);
    dirIterator->setArguments(args);

    fmDebug() << "Iterator arguments set - sortRole:" << static_cast<int>(sortRole)
              << "mixFileAndDir:" << isMixDirAndFile << "sortOrder:" << sortOrder;

    if (!dirIterator->initIterator()) {
        fmWarning() << "dir iterator init failed !! url : " << dirUrl;
        emit traversalFinished(traversalToken);
        return {};
    }
    Q_EMIT iteratorInitFinished();

    // Get the initial list of files
    auto fileList = dirIterator->sortFileInfoList();
    fmInfo() << "Initial file list retrieved - count:" << fileList.size() << "token:" << traversalToken;

    // Emit the initial file list
    emit updateLocalChildren(fileList, sortRole, sortOrder, isMixDirAndFile, traversalToken);

    // Check if the iterator is waiting for more updates (search still in progress, etc.)
    while (dirIterator->isWaitingForUpdates()) {
        fileList = dirIterator->sortFileInfoList();
        if (!fileList.isEmpty())
            emit updateChildrenInfo(fileList, traversalToken);
    }

    // Iterator is not waiting for updates, so signal that we're done
    emit traversalFinished(traversalToken);

    return fileList;
}
