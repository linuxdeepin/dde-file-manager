// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskcommander.h"
#include "taskcommander_p.h"
#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/iterator/iteratorsearcher.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-search/dsearch_global.h>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

SearchResultWorker::SearchResultWorker(QObject *parent)
    : QObject(parent),
      running(true),
      dataProcessingComplete(0),
      allSearchersDone(0)
{
}

SearchResultWorker::~SearchResultWorker()
{
    stop();
}

void SearchResultWorker::startProcessing(const QString &id)
{
    taskId = id;
    running = true;
    
    // Process all queued searchers until stopped
    while (running) {
        processQueue();
        
        // 如果所有搜索器完成且队列为空，执行最终处理
        if (allSearchersDone.loadAcquire() == 1) {
            QMutexLocker lock(&mutex);
            if (searcherQueue.isEmpty()) {
                finalizeProcessing();
                break;
            }
        }
    }
    
    emit processingFinished();
}

void SearchResultWorker::processResults(AbstractSearcher *searcher)
{
    if (!searcher)
        return;
        
    QMutexLocker lock(&mutex);
    searcherQueue.enqueue(searcher);
    condition.wakeOne();
}

void SearchResultWorker::stop()
{
    QMutexLocker lock(&mutex);
    running = false;
    condition.wakeAll();
}

DFMSearchResultMap SearchResultWorker::getResults()
{
    QReadLocker lk(&rwLock);
    return resultMap;
}

void SearchResultWorker::allSearchersFinished()
{
    // 设置标志，指示所有搜索器已完成
    allSearchersDone.storeRelease(1);
    
    // 唤醒工作线程检查队列
    QMutexLocker lock(&mutex);
    condition.wakeOne();
}

void SearchResultWorker::processQueue()
{
    AbstractSearcher *searcher = nullptr;
    
    // Wait for new data with timeout
    {
        QMutexLocker lock(&mutex);
        if (searcherQueue.isEmpty()) {
            // 如果队列为空且所有搜索器已完成，终止处理
            if (allSearchersDone.loadAcquire() == 1) {
                return;
            }
            
            condition.wait(&mutex, 100);  // 带超时等待
            if (searcherQueue.isEmpty())
                return;
        }
        
        searcher = searcherQueue.dequeue();
    }
    
    if (searcher && searcher->hasItem()) {
        // Get and process results from current searcher
        DFMSearchResultMap searchResults = searcher->takeAll();
        
        if (!searchResults.isEmpty()) {
            mergeResults(searchResults);
            emit resultsUpdated(taskId);
        }
    }
}

void SearchResultWorker::mergeResults(const DFMSearchResultMap &newResults)
{
    QWriteLocker lk(&rwLock);
    
    // Merge search results to the main result set
    for (auto it = newResults.constBegin(); it != newResults.constEnd(); ++it) {
        const QUrl &url = it.key();
        
        // If there's already a result for this URL, keep the one with higher match score
        auto existing = resultMap.find(url);
        if (existing != resultMap.end()) {
            if (it.value().matchScore() > existing.value().matchScore())
                resultMap[url] = it.value();
        } else {
            // Otherwise add directly
            resultMap.insert(url, it.value());
        }
    }
}

void SearchResultWorker::finalizeProcessing()
{
    fmInfo() << "Finalizing search result processing for task:" << taskId;
    
    // 标记数据处理已完成
    dataProcessingComplete.storeRelease(1);
    
    // 发送所有处理完成的信号
    emit allProcessingComplete(taskId);
}

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *parent)
    : QObject(parent),
      q(parent),
      finishedCount(0),
      allFinishedNotified(false)
{
    // Create result worker and move it to the worker thread
    resultWorker = new SearchResultWorker;
    resultWorker->moveToThread(&workerThread);
    
    // Connect signals and slots
    connect(&workerThread, &QThread::started, resultWorker, [this]() {
        resultWorker->startProcessing(taskId);
    });
    connect(&workerThread, &QThread::finished, resultWorker, &QObject::deleteLater);
    connect(resultWorker, &SearchResultWorker::resultsUpdated, this, &TaskCommanderPrivate::onResultsUpdated);
    connect(resultWorker, &SearchResultWorker::allProcessingComplete, this, &TaskCommanderPrivate::onAllProcessingComplete);
    
    // Start the worker thread
    workerThread.start();
}

TaskCommanderPrivate::~TaskCommanderPrivate()
{
    // Stop and clean up the worker thread
    if (resultWorker) {
        resultWorker->stop();
    }
    
    workerThread.quit();
    workerThread.wait();
    
    // Clean up searchers
    for (auto searcher : allSearchers) {
        searcher->deleteLater();
    }
    allSearchers.clear();
}

void TaskCommanderPrivate::working(AbstractSearcher *searcher)
{
    searcher->search();
}

AbstractSearcher *TaskCommanderPrivate::createSearcher(const QUrl &url, const QString &keyword, SearchType type)
{
    const char *typeStr = type == SearchType::FileName ? "file name" : "content";
    fmInfo() << "Using dfm-search for" << typeStr << "search";
    auto *searcher = new DFMSearcher(url, keyword, q, type);

    connect(searcher, &AbstractSearcher::unearthed, this, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    connect(searcher, &AbstractSearcher::finished, this, &TaskCommanderPrivate::onFinished, Qt::DirectConnection);
    allSearchers << searcher;

    return searcher;
}

void TaskCommanderPrivate::onUnearthed(AbstractSearcher *searcher)
{
    Q_ASSERT(searcher);

    // Pass the searcher to the worker thread for processing
    if (allSearchers.contains(searcher) && searcher->hasItem() && resultWorker)
        resultWorker->processResults(searcher);
}

void TaskCommanderPrivate::onResultsUpdated(const QString &id)
{
    if (id == taskId) {
        QMetaObject::invokeMethod(q, "matched", Qt::QueuedConnection, Q_ARG(QString, taskId));
    }
}

void TaskCommanderPrivate::onFinished()
{
    if (finishedCount.fetchAndAddRelaxed(1) + 1 == allSearchers.size()) {
        fmInfo() << "All searchers finished for task:" << taskId;
        
        // 通知工作线程所有搜索器已完成
        if (resultWorker) {
            resultWorker->allSearchersFinished();
        } else {
            // 如果没有工作线程，直接检查完成状态
            checkAllFinished();
        }
    }
}

void TaskCommanderPrivate::onAllProcessingComplete(const QString &id)
{
    if (id == taskId && !allFinishedNotified) {
        fmInfo() << "All data processing complete for task:" << taskId;
        allFinishedNotified = true;
        checkAllFinished();
    }
}

void TaskCommanderPrivate::checkAllFinished()
{
    if (deleted) {
        q->deleteLater();
        disconnect(q, nullptr, nullptr, nullptr);
    } else {
        // 只有当所有搜索器完成且数据处理完成，或没有工作线程时，才发送finished信号
        if (!resultWorker || allFinishedNotified) {
            QMetaObject::invokeMethod(q, "finished", Qt::QueuedConnection, Q_ARG(QString, taskId));
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

DFMSearchResultMap TaskCommander::getResults() const
{
    // Get results from the worker thread
    return d->resultWorker ? d->resultWorker->getResults() : DFMSearchResultMap();
}

QList<QUrl> TaskCommander::getResultsUrls() const
{
    // Get result URLs directly from the worker thread
    return d->resultWorker ? d->resultWorker->getResults().keys() : QList<QUrl>();
}

bool TaskCommander::start()
{
    if (d->allSearchers.isEmpty()) {
        fmWarning() << "no searcher...";
        return false;
    }

    for (auto searcher : d->allSearchers) {
        d->working(searcher);
    }

    return true;
}

void TaskCommander::stop()
{
    // Stop all searchers
    for (auto searcher : d->allSearchers) {
        searcher->stop();
    }

    // Stop the result worker
    if (d->resultWorker) {
        d->resultWorker->stop();
    }
}

void TaskCommander::deleteSelf()
{
    d->deleted = true;
    if (d->finishedCount.loadRelaxed() == d->allSearchers.size()) {
        deleteLater();
    }
}

void TaskCommander::createSearcher(const QUrl &url, const QString &keyword)
{
    if (DFMSearcher::supportUrl(url)) {
        // Create file name searcher
        d->createSearcher(url, keyword, SearchType::FileName);

        // Check if full text search is enabled
        bool enableContentSearch = DConfigManager::instance()->value(DConfig::kSearchCfgPath, DConfig::kEnableFullTextSearch, false).toBool();
        if (!enableContentSearch)
            return;

        // Create content searcher
        d->createSearcher(url, keyword, SearchType::Content);
        return;
    }

    // Create iterator searcher
    auto *searcher = new IteratorSearcher(url, keyword);
    connect(searcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    connect(searcher, &AbstractSearcher::finished, d, &TaskCommanderPrivate::onFinished, Qt::DirectConnection);
    d->allSearchers << searcher;
}
