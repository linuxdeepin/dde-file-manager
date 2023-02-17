// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traversaldirthread.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"

#include <QElapsedTimer>
#include <QDebug>

using namespace dfmbase;
USING_IO_NAMESPACE

TraversalDirThread::TraversalDirThread(const QUrl &url,
                                       const QStringList &nameFilters,
                                       QDir::Filters filters,
                                       QDirIterator::IteratorFlags flags,
                                       QObject *parent)
    : QThread(parent), dirUrl(url), nameFilters(nameFilters), filters(filters), flags(flags)
{
    qRegisterMetaType<QList<AbstractFileInfoPointer>>("QList<AbstractFileInfoPointer>");
    qRegisterMetaType<QList<SortInfoPointer>>();
    qRegisterMetaType<SortInfoPointer>();
    if (dirUrl.isValid()) {
        dirIterator = DirIteratorFactory::create<AbstractDirIterator>(url, nameFilters, filters, flags);
        if (!dirIterator) {
            qWarning() << "Failed create dir iterator from" << url;
            return;
        }
    }
}

TraversalDirThread::~TraversalDirThread()
{
    quit();
    wait();
}

void TraversalDirThread::stop()
{
    if (stopFlag)
        return;

    stopFlag = true;
    if (dirIterator)
        dirIterator->close();
}

void TraversalDirThread::quit()
{
    stop();
    QThread::quit();
}

void TraversalDirThread::stopAndDeleteLater()
{
    stop();

    if (!isRunning()) {
        deleteLater();
    } else {
        disconnect(this, &TraversalDirThread::finished, this, &TraversalDirThread::deleteLater);
        connect(this, &TraversalDirThread::finished, this, &TraversalDirThread::deleteLater);
    }
}

void TraversalDirThread::setSortAgruments(const Qt::SortOrder order, const Global::ItemRoles sortRole, const bool isMixDirAndFile)
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

void TraversalDirThread::run()
{
    if (dirIterator.isNull())
        return;

    QElapsedTimer timer;
    timer.start();

    qInfo() << currentThreadId();
    qInfo() << "dir query start, url: " << dirUrl;

    dirIterator->cacheBlockIOAttribute();

    qInfo() << "cacheBlockIOAttribute finished, url: " << dirUrl << " elapsed: " << timer.elapsed();
    if (stopFlag) {
        emit traversalFinished();
        return;
    }

    int count = 0;
    if (!dirIterator->oneByOne()) {
        count = iteratorAll();
        qInfo() << "local dir query end, file count: " << count << " url: " << dirUrl << " elapsed: " << timer.elapsed();
    } else {
        count = iteratorOneByOne();
        qInfo() << "dir query end, file count: " << count << " url: " << dirUrl << " elapsed: " << timer.elapsed();
    }
}

int TraversalDirThread::iteratorOneByOne()
{
    QList<AbstractFileInfoPointer> childrenList;   // 当前遍历出来的所有文件
    while (dirIterator->hasNext()) {
        if (stopFlag)
            break;

        // 调用一次fileinfo进行文件缓存
        const auto &fileUrl = dirIterator->next();
        auto fileInfo = dirIterator->fileInfo();
        if (!fileInfo)
            fileInfo = InfoFactory::create<AbstractFileInfo>(fileUrl);

        if (!fileInfo)
            continue;

        emit updateChild(fileInfo);
        childrenList.append(fileInfo);
    }
    emit updateChildren(childrenList);

    emit traversalFinished();

    return childrenList.count();
}

int TraversalDirThread::iteratorAll()
{
    QVariantMap argus;
    argus.insert("sortRole",
                 QVariant::fromValue(sortRole));
    argus.insert("mixFileAndDir", isMixDirAndFile);
    argus.insert("sortOrder", sortOrder);
    dirIterator->setArguments(argus);
    auto fileList = dirIterator->sortFileInfoList();

    emit updateLocalChildren(fileList, sortRole, sortOrder, isMixDirAndFile);
    emit traversalFinished();

    return fileList.count();
}
