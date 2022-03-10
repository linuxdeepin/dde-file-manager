/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef FILEVIEWMODEL_P_H
#define FILEVIEWMODEL_P_H

#include "models/fileviewmodel.h"
#include "views/fileviewitem.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/traversaldirthread.h"

#include <QReadWriteLock>
#include <QQueue>
#include <QTimer>
#include <QPointer>

using FileNodePointer = QSharedPointer<DPWORKSPACE_NAMESPACE::FileViewItem>;
DPWORKSPACE_BEGIN_NAMESPACE
class FileNodeManagerThread : public QThread
{
    Q_OBJECT
public:
    explicit FileNodeManagerThread(FileViewModel *parent);
    ~FileNodeManagerThread() override;
    inline FileViewModel *model() const
    {
        return static_cast<FileViewModel *>(parent());
    }

    void removeFile(const QUrl &url);
    inline void setRootNode(const FileNodePointer &node)
    {
        root = node;
    }
    void stop();
    void clearChildren();

    FileNodePointer rootNode() const;
    void insertChild(const QUrl &url);
    bool insertChildren(QList<QUrl> &urls);
    void insertAllChildren(const QList<QUrl> &urls);
    void removeChildren(const QUrl &url);
    int childrenCount();
    FileNodePointer childByIndex(const int &index);

    bool fileQueueEmpty();
    int fileQueueCount();
    QUrl dequeueFileQueue();

public slots:
    void onHandleAddFile(const QUrl url);
    void onHandleTraversalFinished();

private:
    void run() override;
    bool insertChildrenByCeiled();

private:
    QQueue<QUrl> fileQueue;
    FileNodePointer root;
    QMutex childrenMutex;
    bool cacheChildren = false;
    QList<FileNodePointer> visibleChildren;
    QMap<QUrl, FileNodePointer> children;
    QMap<QUrl, FileNodePointer> childrenAddMap;
    QMap<QUrl, FileNodePointer> childrenRemoveMap;
    bool stoped { false };
    int timeCeiling { 100 };
    int countCeiling { 1000 };

    QMutex fileQueueMutex;
    QAtomicInteger<bool> isTraversalFinished { false };
};

class FileViewModelPrivate : public QObject
{
    enum EventType {
        kAddFile,
        kRmFile
    };
    Q_OBJECT
    friend class FileViewModel;
    FileViewModel *const q;
    QSharedPointer<FileViewItem> root;

    QSharedPointer<FileNodeManagerThread> nodeManager;

    int column = 0;
    AbstractFileWatcherPointer watcher;
    QPointer<DFMBASE_NAMESPACE::TraversalDirThread> traversalThread;
    bool canFetchMoreFlag = true;
    DFMBASE_NAMESPACE::DThreadList<QUrl> handlingFileList;
    QQueue<QPair<QUrl, EventType>> watcherEvent;
    QMutex watcherEventMutex;
    QAtomicInteger<bool> isUpdatedChildren = false;
    QAtomicInteger<bool> processFileEventRuning = false;
    //文件的刷新队列
    DFMBASE_NAMESPACE::DThreadList<QUrl> updateurlList;
    QTimer updateTimer;

    FileViewModel::State currentState = FileViewModel::Idle;

public:
    explicit FileViewModelPrivate(FileViewModel *qq);
    virtual ~FileViewModelPrivate();
private Q_SLOTS:

    void doFileDeleted(const QUrl &url);

    void dofileAttributeChanged(const QUrl &url, const int &isExternalSource = 1)
    {
        Q_UNUSED(url);
        Q_UNUSED(isExternalSource);
    }

    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl)
    {
        doFileDeleted(fromUrl);
        dofileCreated(toUrl);
    }

    void dofileCreated(const QUrl &url);
    void doFileUpdated(const QUrl &url);
    void doFilesUpdated();
    void dofileClosed(const QUrl &url) { Q_UNUSED(url); }
    void doUpdateChild(const QUrl &child);
    void doWatcherEvent();

private:
    bool checkFileEventQueue();
    //    QString roleDisplayString(int role);
};

DPWORKSPACE_END_NAMESPACE

#endif   // FILEVIEWMODEL_P_H
