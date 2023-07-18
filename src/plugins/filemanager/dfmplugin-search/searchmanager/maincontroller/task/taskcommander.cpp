// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskcommander.h"
#include "taskcommander_p.h"
#include "searchmanager/searcher/fulltext/fulltextsearcher.h"
#include "searchmanager/searcher/anything/anythingsearcher.h"
#include "searchmanager/searcher/iterator/iteratorsearcher.h"
#include "searchmanager/searcher/fsearch/fsearcher.h"

#include <QtConcurrent>

DPSEARCH_USE_NAMESPACE

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

AbstractSearcher *TaskCommanderPrivate::createFileNameSearcher(const QUrl &url, const QString &keyword)
{
    bool isBindPath = false;
    if (AnythingSearcher::isSupported(url, isBindPath)) {
        qInfo() << "Using anything for file name search";
        return new AnythingSearcher(url, keyword, isBindPath, q);
    }

    if (FSearcher::isSupport(url)) {
        qInfo() << "Using fsearch for file name search";
        return new FSearcher(url, keyword, q);
    }

    qInfo() << "Using iterator for file name search";
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

TaskCommander::TaskCommander(QString taskId, const QUrl &url, const QString &keyword, QObject *parent)
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

QList<QUrl> TaskCommander::getResults() const
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
    qInfo() << "stop" << this->taskID();
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

void TaskCommander::createSearcher(const QUrl &url, const QString &keyword)
{
    // 全文搜索
    if (FullTextSearcher::isSupport(url)) {
        FullTextSearcher *searcher = new FullTextSearcher(url, keyword, this);
        //直连，在线程处理
        connect(searcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
        qInfo() << "Using Full-Text search";
        d->allSearchers << searcher;
    }

    // 文件名搜索
    auto *searcher = d->createFileNameSearcher(url, keyword);
    //直连，在线程处理
    connect(searcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    d->allSearchers << searcher;
}
