// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    FileItemData *getChild(int rowIndex);

    void clearChildren();
    void refreshChildren();

public Q_SLOTS:
    void onHandleAddFile(const QUrl url);
    void onHandleTraversalFinished();

Q_SIGNALS:
    void requestSetIdle();

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
