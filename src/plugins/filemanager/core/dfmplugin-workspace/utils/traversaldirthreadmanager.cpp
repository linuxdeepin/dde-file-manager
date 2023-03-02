// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traversaldirthreadmanager.h"
#include "dfm-base/base/schemefactory.h"

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
    qRegisterMetaType<QList<AbstractFileInfoPointer>>();
    qRegisterMetaType<AbstractFileInfoPointer>();
    qRegisterMetaType<QList<SortInfoPointer>>();
    qRegisterMetaType<SortInfoPointer>();
}

TraversalDirThreadManager::~TraversalDirThreadManager()
{
    quit();
    wait();
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

int TraversalDirThreadManager::iteratorOneByOne(const QElapsedTimer &timer)
{
    dirIterator->cacheBlockIOAttribute();
    qInfo() << "cacheBlockIOAttribute finished, url: " << dirUrl << " elapsed: " << timer.elapsed();
    if (stopFlag) {
        emit traversalFinished();
        return 0;
    }

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

        emit updateChildManager(fileInfo);
        childrenList.append(fileInfo);
    }
    emit updateChildrenManager(childrenList);

    emit traversalFinished();

    return childrenList.count();
}

int TraversalDirThreadManager::iteratorAll()
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
