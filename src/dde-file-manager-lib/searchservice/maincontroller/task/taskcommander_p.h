/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
