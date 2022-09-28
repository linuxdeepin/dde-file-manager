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
#ifndef FILEDATACACHETHREAD_H
#define FILEDATACACHETHREAD_H

#include "dfmplugin_workspace_global.h"

#include <QQueue>
#include <QThread>
#include <QMutex>
#include <QReadWriteLock>

namespace dfmplugin_workspace {
class RootInfo;
class FileItemData;
class FileDataCacheThread : public QThread
{
    Q_OBJECT
    friend class RootInfo;

public:
    explicit FileDataCacheThread(RootInfo *r);
    ~FileDataCacheThread() override;

    void stop();

    bool fileQueueEmpty();
    QUrl dequeueFileQueue();

    int childrenCount();
    bool containsChild(const QUrl &url);

    void addChildren(const QList<QUrl> &urls);
    void removeChildren(const QList<QUrl> &urls);

    FileItemData *getChild(int rowIndex) const;

    void clearChildren();
    void refreshChildren();

public Q_SLOTS:
    void onHandleAddFile(const QUrl url);
    void onHandleTraversalFinished();

private:
    void run() override;

private:
    QAtomicInteger<bool> isTraversalFinished { false };
    bool stoped { false };

    QQueue<QUrl> fileQueue;

    QMutex fileQueueMutex;

    QReadWriteLock childrenLock;
    QList<QUrl> childrenUrlList;
    QMap<QUrl, FileItemData *> chilrenDataMap;

    RootInfo *root;
};

typedef QSharedPointer<FileDataCacheThread> CacheThreadPointer;

}

#endif   // FILEDATACACHETHREAD_H
