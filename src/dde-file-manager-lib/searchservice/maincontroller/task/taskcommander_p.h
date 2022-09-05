// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_P_H
#define TASKCOMMANDER_P_H

#include "taskcommander.h"
#include "abstractsearcher.h"

#include <QFutureWatcher>
#include <QUrl>
#include <QReadWriteLock>

class TaskCommanderPrivate : public QObject
{
    Q_OBJECT
    friend class TaskCommander;

public:
    explicit TaskCommanderPrivate(TaskCommander *parent);
    ~TaskCommanderPrivate();

private:
    static void working(AbstractSearcher *searcher);
    AbstractSearcher *createFileNameSearcher(const DUrl &url, const QString &keyword);

private slots:
    void onUnearthed(AbstractSearcher *searcher);
    void onFinished();

private:
    TaskCommander *q = nullptr;
    volatile bool isWorking = false;
    QString taskId;

    //当前所有的搜索结果和新数据缓冲区
    QReadWriteLock rwLock;
    QList<DUrl> resultList;

    bool deleted = false;
    bool finished = false;   //保证结束信号只发一次

    QFutureWatcher<void> futureWatcher;
    QList<AbstractSearcher *> allSearchers;
};

#endif   // TASKCOMMANDER_P_H
