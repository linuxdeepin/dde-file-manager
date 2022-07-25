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
#include "views/fileview.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/traversaldirthread.h"

#include <QReadWriteLock>
#include <QQueue>
#include <QTimer>
#include <QPointer>

using FileNodePointer = QSharedPointer<DPWORKSPACE_NAMESPACE::FileViewItem>;
namespace dfmplugin_workspace {
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

    inline void setRootNode(const FileNodePointer &node)
    {
        root = node;
    }
    FileNodePointer rootNode() const;

    void stop();

    void clearChildren();
    void insertChild(const QUrl &url);
    bool insertChildren(QList<QUrl> &urls);
    void removeChildren(const QUrl &url);

    int childrenCount();
    FileNodePointer childByIndex(const int &index);
    QPair<int, FileNodePointer> childByUrl(const QUrl &url);

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
    FileNodePointer root;

    QQueue<QUrl> fileQueue;
    QList<QUrl> childrenUrlList;
    QMap<QUrl, FileNodePointer> children;

    int timeCeiling { 100 };
    int countCeiling { 1000 };

    QAtomicInteger<bool> isTraversalFinished { false };
    bool stoped { false };

    QMutex fileQueueMutex;
    QMutex childrenMutex;
};

class FileViewModelPrivate : public QObject
{
    Q_OBJECT

    friend class FileViewModel;

    enum EventType {
        kAddFile,
        kRmFile
    };

    FileViewModel *const q;

    QSharedPointer<FileViewItem> rootData;

    AbstractFileWatcherPointer watcher;
    QQueue<QPair<QUrl, EventType>> watcherEvent;
    QMutex watcherEventMutex;

    QPointer<DFMBASE_NAMESPACE::TraversalDirThread> traversalThread;
    QSharedPointer<FileNodeManagerThread> nodeManager;

    bool canFetchMoreFlag = true;
    QAtomicInteger<bool> isUpdatedChildren = false;
    QAtomicInteger<bool> processFileEventRuning = false;

    //文件的刷新队列
    DFMBASE_NAMESPACE::DThreadList<QUrl> updateUrlList;
    QTimer updateTimer;

    FileViewModel::State currentState = FileViewModel::Idle;
    FileView *view { nullptr };

public:
    explicit FileViewModelPrivate(FileViewModel *qq);
    virtual ~FileViewModelPrivate();

private Q_SLOTS:
    void doFileDeleted(const QUrl &url);
    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void dofileCreated(const QUrl &url);
    void doFileUpdated(const QUrl &url);
    void doFilesUpdated();
    void doWatcherEvent();

private:
    bool checkFileEventQueue();
};

}

#endif   // FILEVIEWMODEL_P_H
