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

DPSEARCH_BEGIN_NAMESPACE

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
    void processContentResult(const DFMSEARCH::SearchResult &result);

private:
    TaskCommander *q { nullptr };
    QString taskId { "" };
    QList<AbstractSearcher *> allSearchers {};
    QList<QUrl> resultList {};
    QMap<QUrl, ContentSearchResult> contentResults {};
    QSet<QUrl> processedUrls;  // 记录已处理的 URL
    QReadWriteLock rwLock;
    QAtomicInt finishedCount { 0 };
    bool deleted { false };
};

DPSEARCH_END_NAMESPACE

#endif   // TASKCOMMANDER_P_H
