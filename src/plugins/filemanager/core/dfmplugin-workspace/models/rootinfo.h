// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROOTINFO_H
#define ROOTINFO_H

#include "dfmplugin_workspace_global.h"
#include "utils/filedatacachethread.h"

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/traversaldirthread.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

#include <QReadWriteLock>

namespace dfmplugin_workspace {

class FileItemData;
class RootInfo : public QObject
{
    Q_OBJECT

    enum EventType {
        kAddFile,
        kRmFile
    };

public:
    explicit RootInfo(int i, const QUrl &u, const AbstractFileWatcherPointer &w);

    QList<QUrl> getChildrenUrls() const;
    void clearChildren();
    void startWatcher();
    void refreshChildren();

    int childIndex(const QUrl &url) const;
    int childrenCount();

Q_SIGNALS:
    void insert(int rootIndex, int firstIndex, int count);
    void insertFinish();
    void remove(int rootIndex, int firstIndex, int count);
    void removeFinish();
    void childrenUpdate(const QUrl &url);
    void selectAndEditFile(const QUrl &rootUrl, const QUrl url);
    void reloadView();

public Q_SLOTS:
    void doFileDeleted(const QUrl &url);
    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void dofileCreated(const QUrl &url);
    void doFileUpdated(const QUrl &url);
    void doWatcherEvent();

private:
    bool checkFileEventQueue();
    void enqueueEvent(const QPair<QUrl, EventType> &e);
    QPair<QUrl, EventType> dequeueEvent();

public:

    QUrl url;
    QUrl hiddenFileUrl;
    AbstractFileWatcherPointer watcher;
    TraversalThreadPointer traversal;
    CacheThreadPointer fileCache;
    FileItemData *data;

    QQueue<QPair<QUrl, EventType>> watcherEvent;
    QMutex watcherEventMutex;
    QAtomicInteger<bool> processFileEventRuning = false;
    bool canFetchMore;
    bool needTraversal;
    bool canFetchByForce = false;
    int rowIndex;
};
}

#endif   // ROOTINFO_H
