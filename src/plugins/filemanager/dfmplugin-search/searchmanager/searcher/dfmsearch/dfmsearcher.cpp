// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsearcher.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>

#include <dfm-search/searchfactory.h>

#include <QDebug>

static constexpr int kEmitInterval = 50;   // 推送时间间隔（ms）

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

DFMSearcher::DFMSearcher(const QUrl &url, const QString &keyword, QObject *parent, SearchType type)
    : AbstractSearcher(url, keyword, parent)
{
    engine = SearchFactory::createEngine(type, this);
    if (!engine) {
        fmWarning() << "Failed to create search engine";
        return;
    }

    connect(engine, &SearchEngine::searchStarted, this, &DFMSearcher::onSearchStarted);
    connect(engine, &SearchEngine::resultFound, this, &DFMSearcher::onResultFound);
    connect(engine, &SearchEngine::searchFinished, this, &DFMSearcher::onSearchFinished);
    connect(engine, &SearchEngine::searchCancelled, this, &DFMSearcher::onSearchCancelled);
    connect(engine, &SearchEngine::errorOccurred, this, &DFMSearcher::onSearchError);
}

DFMSearcher::~DFMSearcher()
{
    if (engine) {
        engine->deleteLater();
    }
}

SearchQuery DFMSearcher::createSearchQuery() const
{   
    // 创建搜索查询
    if (!keyword.contains(' '))
        return SearchFactory::createQuery(keyword, SearchQuery::Type::Simple);

    // 如果包含空格，使用布尔查询
    QStringList keywords;
    keywords = keyword.split(' ', Qt::SkipEmptyParts);
    SearchQuery query = SearchFactory::createQuery(keywords, SearchQuery::Type::Boolean);
    // 设置布尔操作符为 AND
    query.setBooleanOperator(SearchQuery::BooleanOperator::AND);
    return query;
}

bool DFMSearcher::search()
{
    if (!engine || engine->status() != SearchStatus::Ready)
        return false;

    const QString &path = UrlRoute::urlToPath(searchUrl);
    if (path.isEmpty() || keyword.isEmpty()) {
        return false;
    }

    notifyTimer.start();

    // 设置搜索选项
    SearchOptions options;
    options.setSearchMethod(SearchMethod::Indexed);
    options.setSearchPath(path);
    options.setCaseSensitive(false);
    options.setIncludeHidden(Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool());
    engine->setSearchOptions(options);

    // 创建并执行搜索查询
    SearchQuery query = createSearchQuery();
    engine->search(query);

    return true;
}

bool DFMSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

DFMSearchResultMap DFMSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

SearchType DFMSearcher::getSearchType() const
{
    return engine ? engine->searchType() : SearchType::FileName;
}

void DFMSearcher::onSearchStarted()
{
    fmInfo() << "Search started for:" << keyword;
}

void DFMSearcher::processSearchResult(const SearchResult &result)
{
    QUrl url = QUrl::fromLocalFile(result.path());
    
    // 创建统一的搜索结果数据结构
    DFMSearchResult searchResult(url);
    
    // 如果是内容搜索，解析高亮内容
    if (engine->searchType() == SearchType::Content) {
        ContentResultAPI contentResult(const_cast<SearchResult &>(result));
        searchResult.setHighlightedContent(contentResult.highlightedContent());
        searchResult.setIsContentMatch(true);
        searchResult.setMatchScore(1.0); // 内容匹配优先级较高
    } else {
        // 文件名搜索不包含高亮内容
        searchResult.setIsContentMatch(false);
        searchResult.setMatchScore(0.5); // 文件名匹配优先级低
    }
    
    // 简化：直接添加/覆盖结果，不需要检查重复项
    // DFMSearcher获取的结果在本实例内可以视作唯一
    QMutexLocker lk(&mutex);
    allResults.insert(url, searchResult);
}

void DFMSearcher::onResultFound(const SearchResult &result)
{
    processSearchResult(result);

    // 每100ms发送一次结果
    if (notifyTimer.elapsed() - lastEmit > 100) {
        lastEmit = notifyTimer.elapsed();
        emit unearthed(this);
    }
}

void DFMSearcher::onSearchFinished(const QList<SearchResult> &results)
{
    if (!engine->searchOptions().resultFoundEnabled()) {
        for (const auto &result : results) {
            processSearchResult(result);
        }
    }

    if (engine->status() == SearchStatus::Finished && hasItem())
        emit unearthed(this);

    fmInfo() << "Search finished, found" << allResults.size() << "results";
    emit finished();
}

void DFMSearcher::onSearchCancelled()
{
    fmInfo() << "Search cancelled";
    emit finished();
}

void DFMSearcher::onSearchError(const SearchError &error)
{
    fmWarning() << "Search error:"
                << "Code:" << error.code().value()
                << "Category:" << error.code().category().name()
                << "Name:" << error.name()
                << "Message:" << error.message();
    emit finished();
} 
