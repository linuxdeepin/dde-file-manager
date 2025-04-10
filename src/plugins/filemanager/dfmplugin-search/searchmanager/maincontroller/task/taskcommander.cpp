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
    
    // 创建统一的DFMSearchResult
    DFMSearchResult searchResult(url);
    searchResult.setHighlightedContent(contentResult.highlightedContent());
    searchResult.setIsContentMatch(true);
    searchResult.setMatchScore(0.5); // 内容匹配的权重
    
    QWriteLocker lk(&rwLock);
    // 检查是否已存在，保留匹配分数更高的
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
        // 获取当前搜索器的结果
        auto searchResults = searcher->takeAll();
        
        // 如果没有结果，不进行处理
        if (searchResults.isEmpty())
            return;
            
        QWriteLocker lk(&rwLock);
        // bool isEmpty = resultMap.isEmpty();
        bool hasNewResults = false;

        // 合并搜索结果到主结果集
        // 这里的合并逻辑是必要的，因为TaskCommander整合了多个搜索器的结果
        for (auto it = searchResults.begin(); it != searchResults.end(); ++it) {
            const QUrl &url = it.key();
            
            // 如果已经添加过该URL，则跳过，避免重复处理
            if (processedUrls.contains(url))
                continue;
                
            processedUrls.insert(url);
            
            // 如果已经有此URL的结果，保留匹配分数更高的
            auto existing = resultMap.find(url);
            if (existing != resultMap.end()) {
                if (it.value().matchScore() > existing.value().matchScore()) {
                    resultMap[url] = it.value();
                }
            } else {
                // 否则直接添加
                resultMap.insert(url, it.value());
                hasNewResults = true;
            }
        }

        // 只有在有新结果时才发送信号
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
    // 创建文件名搜索
    auto *fileNameSearcher = d->createSearcher(url, keyword, SearchType::FileName);
    connect(fileNameSearcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    connect(fileNameSearcher, &AbstractSearcher::finished, d, &TaskCommanderPrivate::onFinished, Qt::DirectConnection);
    d->allSearchers << fileNameSearcher;

    // 检查是否启用全文搜索
    bool enableContentSearch = DConfigManager::instance()->value(DConfig::kSearchCfgPath, DConfig::kEnableFullTextSearch, false).toBool();
    if (!enableContentSearch)
        return;

    // 创建内容搜索
    auto *contentSearcher = d->createSearcher(url, keyword, SearchType::Content);
    connect(contentSearcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    connect(contentSearcher, &AbstractSearcher::finished, d, &TaskCommanderPrivate::onFinished, Qt::DirectConnection);
    d->allSearchers << contentSearcher;
}
