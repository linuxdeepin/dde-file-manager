// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QPointer>

namespace dfmplugin_workspace {

class FileItemData;
class RootInfoWorker;

// 架构说明:
// 本文件包含四个核心类，构成 RootInfo 的遍历-监视体系：
//   - RootInfo          主线程持有，面向 UI 层提供信号接口
//   - RootInfoWorker    运行于 rootThread，持有 childrenUrlList 与 sourceDataList
//   - FileWatcherWorker 运行于 rootThread，处理文件监视器事件
//   - FileIteratorWorker运行于 rootThread，处理目录遍历结果
//
// 线程亲和性:
//   RootInfo 始终在主线程；RootInfoWorker、FileWatcherWorker、FileIteratorWorker
//   均通过 moveToThread 移至 RootInfo 内部的 rootThread。
//
// 数据所有权:
//   childrenUrlList 与 sourceDataList 驻留于 RootInfoWorker（rootThread），
//   仅在 worker 线程中读写，主线程通过 QueuedConnection 信号访问。
//
// 通信方式:
//   主线程 → worker: 通过 Qt::QueuedConnection 信号投递
//   worker → 主线程: 通过 Qt::QueuedConnection 信号投递
//
// stopped 标志:
//   std::atomic_bool，用于在析构/重置时一次性关闭所有 worker 操作，
//   各 worker 在每个入口处检查该标志以快速退出。

class FileWatcherWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcherWorker(RootInfoWorker *root, QObject *parent = nullptr);
    ~FileWatcherWorker();

public Q_SLOTS:
    void doFileDeleted(const QUrl &url);
    void doFileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void doFileCreated(const QUrl &fileUrl);
    void doFileUpdated(const QUrl &fileUrl);

    void doWatcherEvent();
    void doWatcherSubEvent();
    void doCheckAndStartTimer();

private:
    bool isSubFile(const QUrl &fileUrl);

private:
    QPointer<RootInfoWorker> rootptr { nullptr };
    QSet<QUrl> adds, updates, removes;
    bool delayTimeStart { false };
};

class FileIteratorWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileIteratorWorker(RootInfoWorker *root, QObject *parent = nullptr);
    ~FileIteratorWorker();

public slots:
    void handleTraversalResults(const QList<FileInfoPointer> &children, const QString &travseToken);
    void handleTraversalResultsUpdate(const QList<SortInfoPointer> &children, const QString &travseToken, bool increment = false);
    void handleTraversalLocalResult(QList<SortInfoPointer> children,
                                    dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                                    Qt::SortOrder sortOrder,
                                    bool isMixDirAndFile, const QString &travseToken);
    void handleTraversalFinish(const QString &travseToken);

    void handleTraversalSort(const QString &travseToken);

private:
    QPointer<RootInfoWorker> rootptr { nullptr };
};

class RootInfoWorker : public QObject {
    Q_OBJECT
    friend class FileIteratorWorker;
    friend class FileWatcherWorker;
    friend class RootInfo;
public:
    enum IteratorStatus {
        kNone = 0,
        kRunning = 1,
        kFinished = 2,
    };

public:
    explicit RootInfoWorker(const QUrl &url, QObject *parent = nullptr);
    ~RootInfoWorker();
    void stop();

public:
    void addChildren(const QSet<QUrl> &urlList);
    void replaceChildren(const QList<SortInfoPointer> &children);
    void removeChildren(const QSet<QUrl> &urlList);
    FileInfoPointer fileInfo(const QUrl &url);
    void updateChildren(const QSet<QUrl> &urls);

public slots:
    void onResetData();
    void onSetIteratorStatus(const IteratorStatus &status);

Q_SIGNALS:
    void iteratorLocalFiles(const QString &key,
                            const QList<SortInfoPointer> children,
                            const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                            const Qt::SortOrder sortOrder,
                            const bool isMixDirAndFile, const bool isFirst);
    void iteratorUpdateFiles(const QString &key, const QList<SortInfoPointer> children, const bool isFirst);
    void iteratorAddFiles(const QString &key, const QList<SortInfoPointer> sortInfos, const QList<FileInfoPointer> infos);
    void requestSort(const QString &key, const QUrl &dirUrl);
    void traversalFinished(const QString &key);
    void watcherAddFiles(const QList<SortInfoPointer> &children);
    void watcherRemoveFiles(const QList<SortInfoPointer> &children);
    void watcherUpdateFile(const SortInfoPointer sortInfo);
    void watcherUpdateFiles(const QList<SortInfoPointer> &sortInfos);
    void watcherUpdateHideFile(const QUrl &hidUrl);

    void requestCloseTab(const QUrl &url);
    void requestClearRoot(const QUrl &url);
    void renameFileProcessStarted();

private:
    QSharedPointer<FileIteratorWorker> iteratorWorker() const;
    QSharedPointer<FileWatcherWorker> watcherWorker() const;
    SortInfoPointer addChild(const FileInfoPointer &child);
    SortInfoPointer sortFileInfo(const FileInfoPointer &info);
    SortInfoPointer updateChild(const QUrl &url);

private:
    QUrl url;
    QUrl hiddenFileUrl;
    dfmio::DEnumerator::SortRoleCompareFlag originSortRole { dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    Qt::SortOrder originSortOrder { Qt::AscendingOrder };
    bool originMixSort { false };
    IteratorStatus itStatus { IteratorStatus::kNone };
    QList<QUrl> childrenUrlList {};
    QList<SortInfoPointer> sourceDataList {};
    QSharedPointer<FileIteratorWorker> it { nullptr };
    QSharedPointer<FileWatcherWorker> watch { nullptr };
    std::atomic_bool stopped { false };
};


class RootInfo : public QObject
{
    Q_OBJECT

public:
    struct DirIteratorThread
    {
        TraversalThreadManagerPointer traversalThread { nullptr };
    };

public:
    explicit RootInfo(const QUrl &u, QObject *parent = nullptr);
    ~RootInfo();

    void initThreadOfFileData(const QString &key,
                              DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order, bool isMixFileAndFolder);
    void startIteratorWork(const QString &key);
    int clearTraversalThread(const QString &key, const bool isRefresh);

    void setFirstBatch(bool first);
    void reset();

    void addConnectToken(const QString &token);
    QStringList connectTokens() const;

    bool canDelete() const;
    bool checkKeyOnly(const QString &key) const;
    QStringList getKeyWords() const;

Q_SIGNALS:
    void iteratorLocalFiles(const QString &key,
                            const QList<SortInfoPointer> children,
                            const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                            const Qt::SortOrder sortOrder,
                            const bool isMixDirAndFile, bool isFirstBatch = false);
    void iteratorUpdateFiles(const QString &key, const QList<SortInfoPointer> children, bool isFirstBatch = false);
    void iteratorAddFiles(const QString &key, const QList<SortInfoPointer> sortInfos, const QList<FileInfoPointer> infos, bool isFirstBatch = false);
    void watcherAddFiles(const QList<SortInfoPointer> &children);
    void watcherRemoveFiles(const QList<SortInfoPointer> &children);
    void traversalFinished(const QString &key, bool noDataProduced = false);
    void watcherUpdateFile(const SortInfoPointer sortInfo);
    void watcherUpdateFiles(const QList<SortInfoPointer> &sortInfos);
    void watcherUpdateHideFile(const QUrl &hidUrl);
    void requestSort(const QString &key, const QUrl &dirUrl);
    void requestCloseTab(const QUrl &url);

    void renameFileProcessStarted();
    void requestClearRoot(const QUrl &url);

    void resetData();
    void iteratorStatus(const RootInfoWorker::IteratorStatus status);

public Q_SLOTS:
    void handleTraversalFinish(const QString &travseToken);

    void startWatcher();

private:
    void initIteratorConnection(const TraversalThreadManagerPointer &traversalThread);
    void initConnection();
    void clearAllThread();

public:
    AbstractFileWatcherPointer watcher;

private:
    QUrl url;

    QMap<QString, QSharedPointer<DirIteratorThread>> traversalThreads;
    std::atomic_bool isFirstBatch { false };

    QList<TraversalThreadPointer> discardedThread {};
    QList<QSharedPointer<QThread>> threads {};
    std::atomic_bool needStartWatcher { true };
    std::atomic_bool isRefresh { false };
    QStringList connectedTokens;

    QStringList keyWords {};

    QSharedPointer<RootInfoWorker> rootWorker { nullptr };
    QThread rootThread;
};
}

Q_DECLARE_METATYPE(dfmplugin_workspace::RootInfoWorker::IteratorStatus);

#endif   // ROOTINFO_H
