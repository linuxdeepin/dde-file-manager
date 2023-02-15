// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traversaldirthread.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"

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
    qRegisterMetaType<QList<QSharedPointer<DFMIO::DEnumerator::SortFileInfo>>>();
    if (dirUrl.isValid() /*&& !UrlRoute::isVirtual(dirUrl)*/) {
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

    qInfo() << "dir query start, url: " << dirUrl;

    dirIterator->cacheBlockIOAttribute();

    qInfo() << "cacheBlockIOAttribute finished, url: " << dirUrl << " elapsed: " << timer.elapsed();

    // Determine whether the current iterator can be converted to a local iterator and whether it can be removed
    auto localDirIterator = dirIterator.staticCast<LocalDirIterator>();
    if (localDirIterator && FileUtils::isLocalDevice(dirUrl)) {
        QMap<DEnumerator::ArgumentKey, QVariant> argus;
        argus.insert(DEnumerator::ArgumentKey::kArgumentSortRole,
                     QVariant::fromValue(sortRole));
        argus.insert(DEnumerator::ArgumentKey::kArgumentMixDirAndFile, isMixDirAndFile);
        argus.insert(DEnumerator::ArgumentKey::kArgumentSortOrder, sortOrder);
        localDirIterator->setArguments(argus);
        auto fileList = localDirIterator->sortFileInfoList();
//        stopFlag = true;
//        for (auto sortInfo : fileList)
//            childrenList.append(sortInfo->url);

        emit updateLocalChildren(fileList, sortRole, sortOrder, isMixDirAndFile);
        qInfo() << "local dir query end, file count: " << fileList.size() << " url: " << dirUrl << " elapsed: " << timer.elapsed();
//        return;
    }

    if (stopFlag)
        return;

    while (dirIterator->hasNext()) {
        if (stopFlag)
            break;

        // 调用一次fileinfo进行文件缓存
        auto fileInfo = dirIterator->fileInfo();
        const auto &fileUrl = dirIterator->next();
        if (!fileInfo)
            fileInfo = InfoFactory::create<AbstractFileInfo>(fileUrl);

        if (!fileInfo)
            continue;

        emit updateChild(fileInfo);
        childrenList.append(fileInfo);
    }
    stopFlag = true;
    emit updateChildren(childrenList);

    qInfo() << "dir query end, file count: " << childrenList.size() << " url: " << dirUrl << " elapsed: " << timer.elapsed();
}
