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
    explicit RootInfo(int i, const QUrl &u, const AbstractFileWatcherPointer &w, const TraversalThreadPointer &t);

    QList<QUrl> getChildrenUrls() const;
    void clearChildren();
    void startWatcher();
    void refreshChildren();
    void newFileAdded(const QUrl &url);

    int childIndex(const QUrl &url) const;
    int childrenCount();

Q_SIGNALS:
    void insert(int rootIndex, int firstIndex, int count);
    void insertFinish();
    void remove(int rootIndex, int firstIndex, int count);
    void removeFinish();
    void childrenUpdate(const QUrl &url);

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
    int rowIndex;
    bool canFetchMore;
    bool needTraversal;
    bool needInsertRootIndex;
    QUrl url;
    AbstractFileWatcherPointer watcher;
    TraversalThreadPointer traversal;
    CacheThreadPointer fileCache;
    FileItemData *data;

    QQueue<QPair<QUrl, EventType>> watcherEvent;
    QMutex watcherEventMutex;
    QAtomicInteger<bool> processFileEventRuning = false;
};
}

#endif   // ROOTINFO_H
