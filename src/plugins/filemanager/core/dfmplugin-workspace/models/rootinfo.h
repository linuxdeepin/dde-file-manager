// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROOTINFO_H
#define ROOTINFO_H

#include "dfmplugin_workspace_global.h"
#include "utils/traversaldirthreadmanager.h"

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/traversaldirthread.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <QReadWriteLock>
#include <QQueue>
#include <QFuture>

namespace dfmplugin_workspace {

class FileItemData;
class RootInfo : public QObject
{
    Q_OBJECT

    enum EventType {
        kAddFile,
        kUpdateFile,
        kRmFile
    };

public:
    struct DirIteratorThread
    {
        TraversalThreadManagerPointer traversalThread { nullptr };
        // origin data sort information
        dfmio::DEnumerator::SortRoleCompareFlag originSortRole { dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
        Qt::SortOrder originSortOrder { Qt::AscendingOrder };
        bool originMixSort { false };
    };

public:
    explicit RootInfo(const QUrl &u, const bool canCache, QObject *parent = nullptr);
    ~RootInfo();

    bool initThreadOfFileData(const QString &key,
                              DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order, bool isMixFileAndFolder);
    void startWork(const QString &key, const bool getCache = false);
    int clearTraversalThread(const QString &key);

    void reset();

Q_SIGNALS:
    void childrenUpdate(const QUrl &url);

    void itemAdded();
    void iteratorLocalFiles(const QString &key,
                            QList<SortInfoPointer> children,
                            const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                            const Qt::SortOrder sortOrder,
                            const bool isMixDirAndFile);
    void iteratorAddFile(const QString &key, const SortInfoPointer sortInfo, const FileInfoPointer info);
    void iteratorAddFiles(const QString &key, QList<SortInfoPointer> sortInfos, QList<FileInfoPointer> infos);
    void watcherAddFiles(QList<SortInfoPointer> children);
    void watcherRemoveFiles(QList<SortInfoPointer> children);
    void traversalFinished(const QString &key);
    void sourceDatas(const QString &key,
                     QList<SortInfoPointer> children,
                     const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                     const Qt::SortOrder sortOrder,
                     const bool isMixDirAndFile,
                     const bool isFinished);
    void watcherUpdateFile(const SortInfoPointer sortInfo);
    void watcherUpdateHideFile(const QUrl &hidUrl);
    void requestSort(const QString &key);
    void requestCloseTab(const QUrl &url);

public Q_SLOTS:
    void doFileDeleted(const QUrl &url);
    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void dofileCreated(const QUrl &url);
    void doFileUpdated(const QUrl &url);
    void doWatcherEvent();
    void doThreadWatcherEvent();

    void handleTraversalResult(const FileInfoPointer &child, const QString &travseToken);
    void handleTraversalResults(QList<FileInfoPointer> children, const QString &travseToken);
    void handleTraversalLocalResult(QList<SortInfoPointer> children,
                                    dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                                    Qt::SortOrder sortOrder,
                                    bool isMixDirAndFile, const QString &travseToken);
    void handleTraversalFinish(const QString &travseToken);

    void handleTraversalSort(const QString &travseToken);
    void handleGetSourceData(const QString &currentToken);

    void startWatcher();

private:
    void initConnection(const TraversalThreadManagerPointer &traversalThread);

    void addChildren(const QList<QUrl> &urlList);
    void addChildren(const QList<FileInfoPointer> &children);
    void addChildren(const QList<SortInfoPointer> &children);
    SortInfoPointer addChild(const FileInfoPointer &child);
    SortInfoPointer sortFileInfo(const FileInfoPointer &info);
    void removeChildren(const QList<QUrl> &urlList);
    bool containsChild(const QUrl &url);
    void updateChild(const QUrl &url);

    bool checkFileEventQueue();
    void enqueueEvent(const QPair<QUrl, EventType> &e);
    QPair<QUrl, EventType> dequeueEvent();
    FileInfoPointer fileInfo(const QUrl &url);
    QString currentKey(const QString &travseToken);

public:
    AbstractFileWatcherPointer watcher;

private:
    QUrl url;
    QUrl hiddenFileUrl;

    QMap<QString, QSharedPointer<DirIteratorThread>> traversalThreads;
    std::atomic_bool traversalFinish { false };
    std::atomic_bool traversaling { false };

    QReadWriteLock childrenLock;
    QList<QUrl> childrenUrlList {};
    QList<SortInfoPointer> sourceDataList {};
    // origin data sort information
    dfmio::DEnumerator::SortRoleCompareFlag originSortRole { dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    Qt::SortOrder originSortOrder { Qt::AscendingOrder };
    bool originMixSort { false };
    bool canCache { false };

    std::atomic_bool cancelWatcherEvent { false };
    QFuture<void> watcherEventFuture;

    QQueue<QPair<QUrl, EventType>> watcherEvent {};
    QMutex watcherEventMutex;
    QAtomicInteger<bool> processFileEventRuning = false;

    QList<TraversalThreadPointer> discardedThread {};
    QList<QSharedPointer<QThread>> threads {};
};
}

#endif   // ROOTINFO_H
