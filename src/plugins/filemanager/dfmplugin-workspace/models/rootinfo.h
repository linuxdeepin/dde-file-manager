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

namespace dfmplugin_workspace {

class FileItemData;
class RootInfoWorker;

// 处理收到文件监视器信号后，监视事件再另外的线程处理
// 收到一次信号后的200ms内的信号合并，操过200ms的下个200ms再处理
class FileWatcherWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcherWorker(RootInfoWorker *root, QObject *parent = nullptr);
    ~FileWatcherWorker();

public Q_SLOTS:
    // 子线程执行
    void doFileDeleted(const QUrl &url);
    void doFileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void doFileCreated(const QUrl &fileUrl);
    void doFileUpdated(const QUrl &fileUrl);

    // 综合200ms内的文件监视消息合并发送，
    void doWatcherEvent();
    void doWatcherSubEvent();
    void doCheckAndStartTimer();

private:
    // 判断是否是当前目录下的子文件,包含自己
    bool isSubFile(const QUrl &fileUrl);

private:
    RootInfoWorker *rootptr { nullptr };
    QSet<QUrl> adds, updates, removes;
    bool delayTimeStart { false };
};

// 处理迭代器迭代的线程处理worker
class FileIteratorWorker : public QObject
{
    Q_OBJECT
    // 主线程执行
public:
    explicit FileIteratorWorker(RootInfoWorker *root, QObject *parent = nullptr);
    ~FileIteratorWorker();

    // 子线程执行
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
    RootInfoWorker *rootptr { nullptr };
};

// 处理rootinfo业务的线程worker（包括两个业务，文件监视和文件迭代）
class RootInfoWorker : public QObject {
    Q_OBJECT
    friend class FileIteratorWorker;
    friend class FileWatcherWorker;
public:
    enum IteratorStatus {
        kNone = 0, // 没有有开始
        kRunning = 1, // 正在迭代
        kFinished = 2, // 迭代完成
    };

    // 主线程执行
public:
    explicit RootInfoWorker(const QUrl &url, QObject *parent = nullptr);
    ~RootInfoWorker();
    void stop();
    QSharedPointer<FileIteratorWorker> iteratorWorker() const;
    QSharedPointer<FileWatcherWorker> watcherWorker() const;

    // 子线程执行
public:
    void addChildren(const QSet<QUrl> &urlList);
    void addChildren(const QList<SortInfoPointer> &children, const int start = 0);
    SortInfoPointer addChild(const FileInfoPointer &child);
    SortInfoPointer sortFileInfo(const FileInfoPointer &info);
    void removeChildren(const QSet<QUrl> &urlList);
    bool containsChild(const QUrl &url);
    FileInfoPointer fileInfo(const QUrl &url);
    SortInfoPointer updateChild(const QUrl &url);
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
    void iteratorAddFile(const QString &key, const SortInfoPointer sortInfo, const FileInfoPointer info);
    void iteratorAddFiles(const QString &key, const QList<SortInfoPointer> sortInfos, const QList<FileInfoPointer> infos);
    void requestSort(const QString &key, const QUrl &dirUrl);
    void traversalFinished(const QString &key);
    void watcherAddFiles(const QList<SortInfoPointer> &children);
    void watcherRemoveFiles(const QList<SortInfoPointer> &children);
    void watcherUpdateFile(const SortInfoPointer sortInfo);
    void watcherUpdateFiles(const QList<SortInfoPointer> &sortInfos);
    void watcherUpdateHideFile(const QUrl &hidUrl);

    void requestSortDir(const QUrl &dirUrl);
    void requestTreeSortDir(const QString &key, const QUrl &parent);
    void requestCloseTab(const QUrl &url);
    void requestClearRoot(const QUrl &url);
    void renameFileProcessStarted();

    // 发送个rootinfo主线程处理计时器问题
    void watcherTimerStart();

private:
    QUrl url;
    QUrl hiddenFileUrl;
    // origin data sort information
    dfmio::DEnumerator::SortRoleCompareFlag originSortRole { dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareDefault };
    Qt::SortOrder originSortOrder { Qt::AscendingOrder };
    bool originMixSort { false };
    IteratorStatus itStatus { IteratorStatus::kNone }; // 当前迭代器的状态
    // children
    QList<QUrl> childrenUrlList {};
    QList<SortInfoPointer> sourceDataList {};
    //
    QSharedPointer<FileIteratorWorker> it { nullptr };
    QSharedPointer<FileWatcherWorker> watch { nullptr };
    // search keywords
    QStringList keyWords {};
    std::atomic_bool stopped { false };
};


class RootInfo : public QObject
{
    Q_OBJECT

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
    explicit RootInfo(const QUrl &u, QObject *parent = nullptr);
    ~RootInfo();

    void initThreadOfFileData(const QString &key,
                              DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order, bool isMixFileAndFolder);
    void startIteratorWork(const QString &key);
    int clearTraversalThread(const QString &key, const bool isRefresh);

    void setFirstBatch(bool first);
    void reset();

    void addConnectToken(const QString &token) {
        if (connectedTokens.contains(token))
            return;
        connectedTokens << token;
    }
    QStringList connectTokens() const { return connectedTokens; }

    bool canDelete() const;
    bool checkKeyOnly(const QString &key) const;
    QStringList getKeyWords() const;

Q_SIGNALS:
    void itemAdded();
    void iteratorLocalFiles(const QString &key,
                            const QList<SortInfoPointer> children,
                            const dfmio::DEnumerator::SortRoleCompareFlag sortRole,
                            const Qt::SortOrder sortOrder,
                            const bool isMixDirAndFile, bool isFirstBatch = false);
    void iteratorUpdateFiles(const QString &key, const QList<SortInfoPointer> children, bool isFirstBatch = false);
    void iteratorAddFile(const QString &key, const SortInfoPointer sortInfo, const FileInfoPointer info);
    void iteratorAddFiles(const QString &key, const QList<SortInfoPointer> sortInfos, const QList<FileInfoPointer> infos, bool isFirstBatch = false);
    void watcherAddFiles(const QList<SortInfoPointer> &children);
    void watcherRemoveFiles(const QList<SortInfoPointer> &children);
    void traversalFinished(const QString &key, bool noDataProduced = false);
    void watcherUpdateFile(const SortInfoPointer sortInfo);
    void watcherUpdateFiles(const QList<SortInfoPointer> &sortInfos);
    void watcherUpdateHideFile(const QUrl &hidUrl);
    void requestSort(const QString &key, const QUrl &dirUrl);
    void requestCloseTab(const QUrl &url);

    void requestTreeSortDir(const QString &key, const QUrl &parent);
    void renameFileProcessStarted();
    void requestClearRoot(const QUrl &url);

    //发送给worker线程
    void resetData();
    void iteratorStatus(const RootInfoWorker::IteratorStatus status);
    void watcherTimerEvent();

public Q_SLOTS:
    void handleTraversalFinish(const QString &travseToken);
    void onWatcherTimerStart();

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
    std::atomic_bool traversalFinish { false };
    std::atomic_bool traversaling { false };
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
