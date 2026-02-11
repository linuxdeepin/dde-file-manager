// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QMutex>
#include <QTimer>

DPSEARCH_BEGIN_NAMESPACE

// 简化的单一搜索工作线程，负责整个搜索流程
class SimplifiedSearchWorker : public QObject
{
    Q_OBJECT
public:
    explicit SimplifiedSearchWorker(QObject *parent = nullptr);
    ~SimplifiedSearchWorker() override;

    // 设置搜索参数
    Q_INVOKABLE void setTaskId(const QString &id) { taskId = id; }
    Q_INVOKABLE void setSearchUrl(const QUrl &url) { searchUrl = url; }
    Q_INVOKABLE void setKeyword(const QString &keyword) { searchKeyword = keyword; }

    // 获取结果
    Q_INVOKABLE DFMSearchResultMap getResults();
    Q_INVOKABLE QList<QUrl> getResultUrls();

    // 控制搜索流程
    Q_INVOKABLE void startSearch();
    Q_INVOKABLE void stopSearch();

signals:
    void resultsUpdated(const QString &taskId);
    void searchCompleted(const QString &taskId);

protected:
    void run();

private slots:
    void onSearcherFinished();
    void onSearcherUnearthed();

private:
    void createSearchers();
    bool isParentPath(const QString &parentPath, const QString &childPath) const;
    void createSearchersForUrl(const QUrl &url);
    void cleanupSearchers();
    void mergeResults(AbstractSearcher *searcher);

    QString taskId;
    QUrl searchUrl;
    QString searchKeyword;

    QList<AbstractSearcher *> searchers;
    DFMSearchResultMap resultMap;

    QReadWriteLock rwLock;
    QMutex mutex;

    bool isRunning { false };
    int finishedSearcherCount { 0 };
};

class TaskCommanderPrivate : public QObject
{
    Q_OBJECT
    friend class TaskCommander;

public:
    explicit TaskCommanderPrivate(TaskCommander *parent);
    ~TaskCommanderPrivate();

    // 非阻塞终止方法
    // void prepareForDestroy();

private slots:
    void onResultsUpdated(const QString &taskId);
    void onSearchCompleted(const QString &taskId);

private:
    TaskCommander *q { nullptr };
    QString taskId { "" };

    QThread workerThread;
    SimplifiedSearchWorker *searchWorker { nullptr };

    bool deleted { false };
};

DPSEARCH_END_NAMESPACE

#endif   // TASKCOMMANDER_P_H
