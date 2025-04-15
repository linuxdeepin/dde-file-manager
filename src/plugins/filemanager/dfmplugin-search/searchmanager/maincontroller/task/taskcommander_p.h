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

private:
    TaskCommander *q { nullptr };
    QString taskId { "" };
    QList<AbstractSearcher *> allSearchers {};
    DFMSearchResultMap resultMap {};             // 使用统一的结果集
    QReadWriteLock rwLock;
    QAtomicInt finishedCount { 0 };
    bool deleted { false };
};

DPSEARCH_END_NAMESPACE

#endif   // TASKCOMMANDER_P_H
