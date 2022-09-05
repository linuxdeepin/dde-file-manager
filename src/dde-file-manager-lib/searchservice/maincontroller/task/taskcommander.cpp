// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskcommander.h"
#include "taskcommander_p.h"
#include "fulltext/fulltextsearcher.h"
#include "iterator/iteratorsearcher.h"
#include "fsearch/fssearcher.h"

#ifndef DISABLE_QUICK_SEARCH
#include "anything/anythingsearcher.h"
#endif

#include <QtConcurrent>

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *parent)
    : QObject(parent),
      q(parent)
{
}

TaskCommanderPrivate::~TaskCommanderPrivate()
{
}

void TaskCommanderPrivate::working(AbstractSearcher *searcher)
{
    searcher->search();
}

AbstractSearcher *TaskCommanderPrivate::createFileNameSearcher(const DUrl &url, const QString &keyword)
{
#ifndef DISABLE_QUICK_SEARCH
    bool isPrependData = false;
    if (AnythingSearcher::isSupported(url, isPrependData))
        return new AnythingSearcher(url, keyword, isPrependData, q);
#endif

    if (FsSearcher::isSupported(url))
        return new FsSearcher(url, keyword, q);

    return new IteratorSearcher(url, keyword, q);
}

void TaskCommanderPrivate::onUnearthed(AbstractSearcher *searcher)
{
    Q_ASSERT(searcher);

    if (allSearchers.contains(searcher) && searcher->hasItem()) {
        auto results = searcher->takeAll();
        QWriteLocker lk(&rwLock);
        bool isEmpty = resultList.isEmpty();

        resultList += results;
        //回到主线程发送信号
        if (isEmpty)
            QMetaObject::invokeMethod(q, "matched", Qt::QueuedConnection, Q_ARG(QString, taskId));
    }
}

void TaskCommanderPrivate::onFinished()
{
    qDebug() << __FUNCTION__ << allSearchers.size() << finished << sender();
    // 工作线程退出，若之前调用了deleteSelf那么在这里执行释放，否则发送结束信号
    if (futureWatcher.isFinished()) {
        if (deleted) {
            q->deleteLater();
            disconnect(q, nullptr, nullptr, nullptr);
        } else if (!finished) {
            finished = true;
            emit q->finished(taskId);
        }
    }
}

TaskCommander::TaskCommander(QString taskId, const DUrl &url, const QString &keyword, QObject *parent)
    : QObject(parent),
      d(new TaskCommanderPrivate(this))
{
    d->taskId = taskId;
    createSearcher(url, keyword);
}

QString TaskCommander::taskID() const
{
    return d->taskId;
}

QList<DUrl> TaskCommander::getResults() const
{
    QReadLocker lk(&d->rwLock);
    return std::move(d->resultList);
}

bool TaskCommander::start()
{
    if (d->isWorking)
        return false;

    d->isWorking = true;
    bool isOn = false;

    // 所有搜索项在线程池中执行
    if (!d->allSearchers.isEmpty()) {
        d->futureWatcher.setFuture(QtConcurrent::map(d->allSearchers, TaskCommanderPrivate::working));
        connect(&d->futureWatcher, &QFutureWatcherBase::finished, d, &TaskCommanderPrivate::onFinished);
        isOn = true;
    }

    // 无工作对象，直接结束。
    if (!isOn) {
        d->isWorking = false;
        qWarning() << __FUNCTION__ << "no searcher...";
        // 加入队列，在start函数返回后发送结束信号
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(QString, d->taskId));
    }

    return true;
}

void TaskCommander::stop()
{
    qDebug() << "stop" << this->taskID();
    d->futureWatcher.cancel();

    for (auto searcher : d->allSearchers) {
        Q_ASSERT(searcher);
        searcher->stop();
    }

    d->isWorking = false;
    d->finished = true;
}

void TaskCommander::deleteSelf()
{
    if (d->futureWatcher.isFinished())
        delete this;
    else
        d->deleted = true;
}

void TaskCommander::createSearcher(const DUrl &url, const QString &keyword)
{
    // 全文搜索
    if (FullTextSearcher::isSupport(url)) {
        FullTextSearcher *searcher = new FullTextSearcher(url, keyword, this);
        //直连，在线程处理
        connect(searcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
        d->allSearchers << searcher;
    }

    // 文件名搜索
    auto *searcher = d->createFileNameSearcher(url, keyword);
    //直连，在线程处理
    connect(searcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    d->allSearchers << searcher;
}
