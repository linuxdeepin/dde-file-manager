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
    int clearTraversalThread(const QString &key, const bool isRefresh);

    void reset();

    void addConnectToken(const QString &token)
    {
        if (connectedTokens.contains(token))
            return;
        connectedTokens << token;
    }
    QStringList connectTokens() const { return connectedTokens; }

    bool canDelete() const;

Q_SIGNALS:

    void itemAdded();
    void iteratorLocalFiles(const QString &key,
                            const QList<SortInfoPointer> children,
                            const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                            const Qt::SortOrder sortOrder,
                            const bool isMixDirAndFile);
    void iteratorAddFile(const QString &key, const SortInfoPointer sortInfo, const FileInfoPointer info);
    void iteratorAddFiles(const QString &key, const QList<SortInfoPointer> sortInfos, const QList<FileInfoPointer> infos);
    void watcherAddFiles(const QList<SortInfoPointer> &children);
    void watcherRemoveFiles(const QList<SortInfoPointer> &children);
    void traversalFinished(const QString &key);
    void sourceDatas(const QString &key,
                     const QList<SortInfoPointer> children,
                     const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                     const Qt::SortOrder sortOrder,
                     const bool isMixDirAndFile,
                     const bool isFinished);
    void watcherUpdateFile(const SortInfoPointer sortInfo);
    void watcherUpdateFiles(const QList<SortInfoPointer> &sortInfos);
    void watcherUpdateHideFile(const QUrl &hidUrl);
    void requestSort(const QString &key, const QUrl &dirUrl);
    void requestCloseTab(const QUrl &url);

    void requestTreeSortDir(const QString &key, const QUrl &parent);
    void renameFileProcessStarted();
    void requestClearRoot(const QUrl &url);

public Q_SLOTS:
    void doFileDeleted(const QUrl &url);
    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void dofileCreated(const QUrl &url);
    void doFileUpdated(const QUrl &url);
    void doWatcherEvent();
    void doThreadWatcherEvent();

    void handleTraversalResult(const FileInfoPointer &child, const QString &travseToken);
    void handleTraversalResults(const QList<FileInfoPointer> children, const QString &travseToken);
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
    SortInfoPointer updateChild(const QUrl &url);
    void updateChildren(const QList<QUrl> &urls);

    bool checkFileEventQueue();
    void enqueueEvent(const QPair<QUrl, EventType> &e);
    QPair<QUrl, EventType> dequeueEvent();
    FileInfoPointer fileInfo(const QUrl &url);

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
    QList<QFuture<void>> watcherEventFutures;

    QQueue<QPair<QUrl, EventType>> watcherEvent {};
    QMutex watcherEventMutex;
    QAtomicInteger<bool> processFileEventRuning = false;

    QList<TraversalThreadPointer> discardedThread {};
    QList<QSharedPointer<QThread>> threads {};
    std::atomic_bool needStartWatcher { true };
    std::atomic_bool isRefresh { false };
    QStringList connectedTokens;
};
}

#endif   // ROOTINFO_H
