/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

QUrl FileDataCacheThread::dequeueFileQueue()
{
    QMutexLocker lk(&fileQueueMutex);
    if (fileQueue.isEmpty())
        return QUrl();
    return fileQueue.dequeue();
}

int FileDataCacheThread::childrenCount()
{
    QReadLocker lk(&childrenLock);
    return childrenUrlList.count();
}

void FileDataCacheThread::onHandleAddFile(const QUrl url)
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

void FileDataCacheThread::onHandleTraversalFinished()
{
    isTraversalFinished = true;
    QApplication::restoreOverrideCursor();
}

void FileDataCacheThread::run()
{
    QUrl url;
    QList<QUrl> needInsertList;

    while (!fileQueueEmpty() || !isTraversalFinished) {
        if (stoped)
            return;

        url = dequeueFileQueue();
        if (!url.isValid())
            continue;

        if (!isTraversalFinished) {
            addChildren({ url });
        } else if (isTraversalFinished && !fileQueueEmpty()) {
            needInsertList.append(url);
        } else {
            needInsertList.append(url);
        }
    }

    if (needInsertList.count() > 0)
        addChildren(needInsertList);
}

void FileDataCacheThread::addChildren(const QList<QUrl> &urls)
{
    int count = childrenCount();

    root->insert(root->rowIndex, count, urls.count());

    QWriteLocker lk(&childrenLock);
    for (const QUrl &url : urls) {
        childrenUrlList.append(url);
        FileItemData *data = new FileItemData(url);
        data->setParentData(root->data);
        chilrenDataMap.insert(url, data);
    }
    lk.unlock();

    root->insertFinish();
}

void FileDataCacheThread::removeChildren(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
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
        chilrenDataMap.remove(url);
        lk.unlock();
        root->removeFinish();
    }
}

FileItemData *FileDataCacheThread::getChild(int rowIndex) const
{
    return chilrenDataMap[childrenUrlList.at(rowIndex)];
}

void FileDataCacheThread::clearChildren()
{
    stop();

    int childCount = childrenCount();

    root->remove(root->rowIndex, 0, childCount);
    QWriteLocker lk(&childrenLock);
    childrenUrlList.clear();
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
