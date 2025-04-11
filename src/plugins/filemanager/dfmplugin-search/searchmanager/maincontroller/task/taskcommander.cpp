// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskcommander.h"
#include "taskcommander_p.h"
#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-search/dsearch_global.h>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *parent)
    : QObject(parent),
      q(parent),
      finishedCount(0)
{
}

TaskCommanderPrivate::~TaskCommanderPrivate()
{
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
    return new DFMSearcher(url, keyword, q, type);
}

void TaskCommanderPrivate::processContentResult(const SearchResult &result)
{
    ContentResultAPI contentResult(const_cast<SearchResult &>(result));
    QUrl url = QUrl::fromLocalFile(result.path());
    
    // Create unified DFMSearchResult
    DFMSearchResult searchResult(url);
    searchResult.setHighlightedContent(contentResult.highlightedContent());
    searchResult.setIsContentMatch(true);
    searchResult.setMatchScore(0.5); // Weight for content match
    
    QWriteLocker lk(&rwLock);
    // Check if already exists, keep the one with higher match score
    auto it = resultMap.find(url);
    if (it != resultMap.end()) {
        if (searchResult.matchScore() > it.value().matchScore()) {
            resultMap[url] = searchResult;
        }
    } else {
        resultMap.insert(url, searchResult);
    }
}

void TaskCommanderPrivate::onUnearthed(AbstractSearcher *searcher)
{
    Q_ASSERT(searcher);

    if (allSearchers.contains(searcher) && searcher->hasItem()) {
        // Get results from current searcher
        auto searchResults = searcher->takeAll();
        
        // If no results, do not process
        if (searchResults.isEmpty())
            return;
            
        QWriteLocker lk(&rwLock);
        bool hasNewResults = false;

        // Merge search results to the main result set
        // This merge logic is necessary because TaskCommander integrates results from multiple searchers
        for (auto it = searchResults.begin(); it != searchResults.end(); ++it) {
            const QUrl &url = it.key();
            
            // If this URL has been processed, skip to avoid duplicate processing
            if (processedUrls.contains(url))
                continue;
                
            processedUrls.insert(url);
            
            // If there's already a result for this URL, keep the one with higher match score
            auto existing = resultMap.find(url);
            if (existing != resultMap.end()) {
                if (it.value().matchScore() > existing.value().matchScore()) {
                    resultMap[url] = it.value();
                }
            } else {
                // Otherwise add directly
                resultMap.insert(url, it.value());
                hasNewResults = true;
            }
        }

        // Only send signal when there are new results
        if (hasNewResults)
            QMetaObject::invokeMethod(q, "matched", Qt::QueuedConnection, Q_ARG(QString, taskId));
    }
}

void TaskCommanderPrivate::onFinished()
{
    if (finishedCount.fetchAndAddRelaxed(1) + 1 == allSearchers.size()) {
        checkAllFinished();
    }
}

void TaskCommanderPrivate::checkAllFinished()
{
    if (deleted) {
        q->deleteLater();
        disconnect(q, nullptr, nullptr, nullptr);
    } else {
        QMetaObject::invokeMethod(q, "finished", Qt::QueuedConnection, Q_ARG(QString, taskId));
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
    QReadLocker lk(&d->rwLock);
    return d->resultMap;
}

QList<QUrl> TaskCommander::getResultsUrls() const
{
    QReadLocker lk(&d->rwLock);
    return d->resultMap.keys();
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

void TaskCommander::deleteSelf()
{
    d->deleted = true;
    if (d->finishedCount.loadRelaxed() == d->allSearchers.size()) {
        deleteLater();
    }
}

void TaskCommander::createSearcher(const QUrl &url, const QString &keyword)
{
    // Create file name searcher
    auto *fileNameSearcher = d->createSearcher(url, keyword, SearchType::FileName);
    connect(fileNameSearcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    connect(fileNameSearcher, &AbstractSearcher::finished, d, &TaskCommanderPrivate::onFinished, Qt::DirectConnection);
    d->allSearchers << fileNameSearcher;

    // Check if full text search is enabled
    bool enableContentSearch = DConfigManager::instance()->value(DConfig::kSearchCfgPath, DConfig::kEnableFullTextSearch, false).toBool();
    if (!enableContentSearch)
        return;

    // Create content searcher
    auto *contentSearcher = d->createSearcher(url, keyword, SearchType::Content);
    connect(contentSearcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    connect(contentSearcher, &AbstractSearcher::finished, d, &TaskCommanderPrivate::onFinished, Qt::DirectConnection);
    d->allSearchers << contentSearcher;
}
