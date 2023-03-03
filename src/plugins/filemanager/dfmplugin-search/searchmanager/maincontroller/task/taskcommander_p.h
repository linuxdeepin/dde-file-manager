// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_P_H
#define TASKCOMMANDER_P_H

#include "taskcommander.h"
#include "searchmanager/searcher/abstractsearcher.h"

#include <QFutureWatcher>
#include <QUrl>
#include <QReadWriteLock>

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
    AbstractSearcher *createFileNameSearcher(const QUrl &url, const QString &keyword);

private slots:
    void onUnearthed(AbstractSearcher *searcher);
    void onFinished();

private:
    TaskCommander *q = nullptr;
    volatile bool isWorking = false;
    QString taskId;

    //当前所有的搜索结果和新数据缓冲区
    QReadWriteLock rwLock;
    QList<QUrl> resultList;

    bool deleted = false;
    bool finished = false;   //保证结束信号只发一次

    QFutureWatcher<void> futureWatcher;
    QList<AbstractSearcher *> allSearchers;
};

DPSEARCH_END_NAMESPACE

#endif   // TASKCOMMANDER_P_H
