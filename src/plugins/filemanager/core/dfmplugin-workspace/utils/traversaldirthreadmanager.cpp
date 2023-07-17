// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traversaldirthreadmanager.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <QElapsedTimer>
#include <QDebug>

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
    qRegisterMetaType<QList<FileInfoPointer>>();
    qRegisterMetaType<FileInfoPointer>();
    qRegisterMetaType<QList<SortInfoPointer>>();
    qRegisterMetaType<SortInfoPointer>();
}

TraversalDirThreadManager::~TraversalDirThreadManager()
{
    quit();
    wait();
    if (future) {
        future->deleteLater();
        future = nullptr;
    }
}

void TraversalDirThreadManager::setSortAgruments(const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile)
{
    sortOrder = order;
    this->isMixDirAndFile = isMixDirAndFile;
    switch (sortRole) {
    case Global::ItemRoles::kItemFileDisplayNameRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileName;
        break;
    case Global::ItemRoles::kItemFileSizeRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileSize;
        break;
    case Global::ItemRoles::kItemFileLastReadRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastRead;
        break;
    case Global::ItemRoles::kItemFileLastModifiedRole:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileLastModified;
        break;
    default:
        this->sortRole = dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault;
    }
}

void TraversalDirThreadManager::start()
{
    auto local = dirIterator.dynamicCast<LocalDirIterator>();
    if (local && local->oneByOne()) {
        future = local->asyncIterator();
        if (future) {
            connect(future, &DEnumeratorFuture::asyncIteratorOver, this, &TraversalDirThreadManager::onAsyncIteratorOver);
            future->startAsyncIterator();
            return;
        }
    }

    Q_EMIT iteratorInitFinished();
    TraversalDirThread::start();
}

void TraversalDirThreadManager::onAsyncIteratorOver()
{
    Q_EMIT iteratorInitFinished();
    TraversalDirThread::start();
}

void TraversalDirThreadManager::run()
{
    if (dirIterator.isNull())
        return;

    QElapsedTimer timer;
    timer.start();
    qInfo() << "dir query start, url: " << dirUrl;

    int count = 0;
    if (!dirIterator->oneByOne()) {
        count = iteratorAll();
        qInfo() << "local dir query end, file count: " << count << " url: " << dirUrl << " elapsed: " << timer.elapsed();
    } else {
        count = iteratorOneByOne(timer);
        qInfo() << "dir query end, file count: " << count << " url: " << dirUrl << " elapsed: " << timer.elapsed();
    }
}

int TraversalDirThreadManager::iteratorOneByOne(const QElapsedTimer &timere)
{
    dirIterator->cacheBlockIOAttribute();
    qInfo() << "cacheBlockIOAttribute finished, url: " << dirUrl << " elapsed: " << timere.elapsed();
    if (stopFlag) {
        emit traversalFinished();
        return 0;
    }

    if (!timer)
        timer = new QElapsedTimer();

    timer->restart();

    QList<FileInfoPointer> childrenList;   // 当前遍历出来的所有文件
    while (dirIterator->hasNext()) {
        if (stopFlag)
            break;

        // 调用一次fileinfo进行文件缓存
        const auto &fileUrl = dirIterator->next();
        if (!fileUrl.isValid())
            continue;
        auto fileInfo = dirIterator->fileInfo();
        if (fileUrl.isValid() && !fileInfo)
            fileInfo = InfoFactory::create<FileInfo>(fileUrl);

        if (!fileInfo)
            continue;

        childrenList.append(fileInfo);

        if (timer->elapsed() > timeCeiling || childrenList.count() > countCeiling) {
            emit updateChildrenManager(childrenList);
            timer->restart();
            childrenList.clear();
        }
    }

    if (childrenList.length() > 0)
        emit updateChildrenManager(childrenList);

    emit traversalRequestSort();

    emit traversalFinished();

    return childrenList.count();
}

int TraversalDirThreadManager::iteratorAll()
{
    QVariantMap args;
    args.insert("sortRole",
                QVariant::fromValue(sortRole));
    args.insert("mixFileAndDir", isMixDirAndFile);
    args.insert("sortOrder", sortOrder);
    dirIterator->setArguments(args);
    auto fileList = dirIterator->sortFileInfoList();

    emit updateLocalChildren(fileList, sortRole, sortOrder, isMixDirAndFile);
    emit traversalFinished();

    return fileList.count();
}
