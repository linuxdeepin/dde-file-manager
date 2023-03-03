// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedatacachethread.h"
#include "models/rootinfo.h"
#include "models/fileitemdata.h"

using namespace dfmplugin_workspace;

#include <QApplication>
#include <QDebug>

FileDataCacheThread::FileDataCacheThread(RootInfo *r)
    : root(r)
{
}

FileDataCacheThread::~FileDataCacheThread()
{
    stop();
}

void FileDataCacheThread::stop()
{
    {
        QMutexLocker lk(&fileQueueMutex);
        fileQueue.clear();
    }

    stoped = true;
    wait();
    isTraversalFinished = false;
    stoped = false;

    QApplication::restoreOverrideCursor();
}

bool FileDataCacheThread::fileQueueEmpty()
{
    QMutexLocker lk(&fileQueueMutex);
    return fileQueue.isEmpty();
}

AbstractFileInfoPointer FileDataCacheThread::dequeueFileQueue()
{
    QMutexLocker lk(&fileQueueMutex);
    if (fileQueue.isEmpty())
        return AbstractFileInfoPointer{};
    return fileQueue.dequeue();
}

int FileDataCacheThread::childrenCount()
{
    QReadLocker lk(&childrenLock);
    return childrenUrlList.count();
}

bool FileDataCacheThread::containsChild(const QUrl &url)
{
    QReadLocker lk(&childrenLock);
    return chilrenDataMap.contains(url);
}

void FileDataCacheThread::onHandleAddFile(const AbstractFileInfoPointer child)
{
    {
        QMutexLocker lk(&fileQueueMutex);
        fileQueue.enqueue(child);
    }

    if (!isRunning()) {
        stoped = false;
        start();
    }
}

void FileDataCacheThread::onHandleTraversalFinished()
{
    isTraversalFinished = true;
    QApplication::restoreOverrideCursor();
}

void FileDataCacheThread::run()
{
    AbstractFileInfoPointer child;
    QList<AbstractFileInfoPointer> needInsertList;

    while (!fileQueueEmpty() || !isTraversalFinished) {
        if (stoped)
            return;

        child = dequeueFileQueue();
        if (!child)
            continue;

        if (!isTraversalFinished) {
            addChildren({ child });
        } else if (isTraversalFinished && !fileQueueEmpty()) {
            needInsertList.append(child);
        } else {
            needInsertList.append(child);
        }
    }

    if (needInsertList.count() > 0)
        addChildren(needInsertList);

    Q_EMIT requestSetIdle();
}

void FileDataCacheThread::addChildren(const QList<AbstractFileInfoPointer> &children)
{
    int count = childrenCount();

    for (const auto &child : children) {
        if (stoped)
            return;

        if (!child)
            continue;

        auto url = child->urlOf(dfmbase::UrlInfoType::kUrl);
        url.setPath(url.path());

        if (containsChild(url))
            continue;

        FileItemData *data = new FileItemData(url, child);
        data->setParentData(root->data);

        root->insert(root->rowIndex, count, 1);

        QWriteLocker lk(&childrenLock);
        childrenUrlList.append(url);
        chilrenDataMap.insert(url, data);
        lk.unlock();

        root->insertFinish();

        ++count;
    }
}

void FileDataCacheThread::addChildrenByUrl(const QList<QUrl> &children)
{
    int count = childrenCount();

    for (auto url : children) {
        if (stoped)
            return;

        if (!url.isValid())
            continue;

        url.setPath(url.path());

        if (containsChild(url))
            continue;

        FileItemData *data = new FileItemData(url);
        data->setParentData(root->data);

        root->insert(root->rowIndex, count, 1);

        QWriteLocker lk(&childrenLock);
        childrenUrlList.append(url);
        chilrenDataMap.insert(url, data);
        lk.unlock();

        root->insertFinish();

        ++count;
    }
}

void FileDataCacheThread::removeChildren(const QList<QUrl> &urls)
{
    for (QUrl url : urls) {
        url.setPath(url.path());
        int childIndex = -1;
        {
            QReadLocker lk(&childrenLock);
            childIndex = childrenUrlList.indexOf(url);
        }

        if (childIndex == -1)
            continue;

        root->remove(root->rowIndex, childIndex, 1);
        QWriteLocker lk(&childrenLock);
        childrenUrlList.removeOne(url);
        chilrenDataMap[url]->deleteLater();
        chilrenDataMap.remove(url);
        lk.unlock();
        root->removeFinish();
    }
}

FileItemData *FileDataCacheThread::getChild(int rowIndex)
{
    if (rowIndex >= 0 && rowIndex < childrenCount()) {
        QReadLocker lk(&childrenLock);
        return chilrenDataMap[childrenUrlList.at(rowIndex)];
    }

    return nullptr;
}

void FileDataCacheThread::clearChildren()
{
    stop();

    int childCount = childrenCount();

    root->remove(root->rowIndex, 0, childCount);
    QWriteLocker lk(&childrenLock);
    childrenUrlList.clear();
    qDeleteAll(chilrenDataMap.values());
    chilrenDataMap.clear();
    lk.unlock();
    root->removeFinish();
}

void FileDataCacheThread::refreshChildren()
{
    QMap<QUrl, FileItemData *>::const_iterator iter = chilrenDataMap.begin();
    while (iter != chilrenDataMap.end()) {
        iter.value()->refreshInfo();
        iter++;
    }
}
