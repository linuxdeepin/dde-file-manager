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
    ContentSearchResult searchResult;
    searchResult.url = QUrl::fromLocalFile(result.path());
    searchResult.highlightedContent = contentResult.highlightedContent();

    QWriteLocker lk(&rwLock);
    contentResults.insert(searchResult.url, searchResult);
}

void TaskCommanderPrivate::onUnearthed(AbstractSearcher *searcher)
{
    Q_ASSERT(searcher);

    if (allSearchers.contains(searcher) && searcher->hasItem()) {
        auto results = searcher->takeAll();
        QWriteLocker lk(&rwLock);
        bool isEmpty = resultList.isEmpty();
        bool hasNewResults = false;

        // 去重处理
        for (const auto &url : results) {
            if (processedUrls.contains(url))
                continue;

            processedUrls.insert(url);
            resultList.append(url);
            hasNewResults = true;
        }

        // 只有在有新结果时才发送信号
        if (hasNewResults && isEmpty)
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

QList<QUrl> TaskCommander::getResults() const
{
    QReadLocker lk(&d->rwLock);
    return d->resultList;
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
