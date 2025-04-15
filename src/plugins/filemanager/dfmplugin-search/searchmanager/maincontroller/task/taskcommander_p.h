// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_P_H
#define TASKCOMMANDER_P_H

#include "taskcommander.h"
#include "searchmanager/searcher/abstractsearcher.h"

#include <dfm-search/dsearch_global.h>
#include <dfm-search/contentsearchapi.h>

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <QAtomicInt>
#include <QMap>
#include <QSet>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

DPSEARCH_BEGIN_NAMESPACE

// New worker class for processing search results in a separate thread
class SearchResultWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchResultWorker(QObject *parent = nullptr);
    ~SearchResultWorker() override;

    void processResults(AbstractSearcher *searcher);
    void stop();
    DFMSearchResultMap getResults();
    
    // 标记所有搜索器已完成，通知工作线程最终处理并等待
    void allSearchersFinished();

signals:
    void resultsUpdated(const QString &taskId);
    void processingFinished();
    void allProcessingComplete(const QString &taskId);

public slots:
    void startProcessing(const QString &taskId);

private:
    void processQueue();
    void mergeResults(const DFMSearchResultMap &newResults);
    void finalizeProcessing();  // 完成最终处理，清理资源

    QString taskId;
    QQueue<AbstractSearcher*> searcherQueue;
    DFMSearchResultMap resultMap;
    QMutex mutex;
    QWaitCondition condition;
    QReadWriteLock rwLock;
    bool running { true };
    QAtomicInt dataProcessingComplete { 0 };  // 标记数据处理是否完成
    QAtomicInt allSearchersDone { 0 };        // 标记所有搜索器是否完成
};

class TaskCommanderPrivate : public QObject
{
    Q_OBJECT
    friend class TaskCommander;

public:
    explicit TaskCommanderPrivate(TaskCommander *parent);
    ~TaskCommanderPrivate();

private:
    static void working(AbstractSearcher *searcher);
    AbstractSearcher *createSearcher(const QUrl &url, const QString &keyword, DFMSEARCH::SearchType type);

private slots:
    void onUnearthed(AbstractSearcher *searcher);
    void onFinished();
    void checkAllFinished();
    void onResultsUpdated(const QString &taskId);
    void onAllProcessingComplete(const QString &taskId);

private:
    TaskCommander *q { nullptr };
    QString taskId { "" };
    QList<AbstractSearcher *> allSearchers {};
    QThread workerThread;
    SearchResultWorker *resultWorker { nullptr };
    QReadWriteLock rwLock;
    QAtomicInt finishedCount { 0 };
    bool deleted { false };
    bool allFinishedNotified { false };  // 防止重复发送finished信号
};

DPSEARCH_END_NAMESPACE

#endif   // TASKCOMMANDER_P_H
