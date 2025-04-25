// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsearcher.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>

#include <dfm-search/searchfactory.h>

#include <QDebug>

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
    // connect(engine, &SearchEngine::resultFound, this, [this](const SearchResult &result) {
    //     // 直接处理搜索结果，不再使用后台处理线程
    //     processSearchResult(result);

    //     // 如果找到了结果，发出信号通知
    //     if (!allResults.isEmpty() && allResults.size() % kBatchSize == 0) {
    //         emit unearthed(this);
    //     }
    // });
    connect(engine, &SearchEngine::searchFinished, this, &DFMSearcher::onSearchFinished);
    connect(engine, &SearchEngine::searchCancelled, this, &DFMSearcher::onSearchCancelled);
    connect(engine, &SearchEngine::errorOccurred, this, &DFMSearcher::onSearchError);
}

DFMSearcher::~DFMSearcher()
{
}

bool DFMSearcher::supportUrl(const QUrl &url)
{
    return url.scheme() == "file";
}

SearchQuery DFMSearcher::createSearchQuery() const
{
    // Create search query
    if (!keyword.contains(' '))
        return SearchFactory::createQuery(keyword, SearchQuery::Type::Simple);

    // If contains spaces, use boolean query
    QStringList keywords;
    keywords = keyword.split(' ', Qt::SkipEmptyParts);
    SearchQuery query = SearchFactory::createQuery(keywords, SearchQuery::Type::Boolean);
    // Set boolean operator to AND
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

    // Transform the path using FileUtils::bindPathTransform to handle bind mounts
    const QString transformedPath = FileUtils::bindPathTransform(path, false);

    // Set search options
    SearchOptions options;
    options.setSearchMethod(SearchMethod::Indexed);
    options.setSearchPath(transformedPath);
    options.setCaseSensitive(false);
    options.setIncludeHidden(Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool());

    if (engine->searchType() == SearchType::Content) {
        ContentOptionsAPI contentAPI(options);
        contentAPI.setMaxPreviewLength(200);
    }

    engine->setSearchOptions(options);

    // Create and execute search query
    SearchQuery query = createSearchQuery();
    engine->search(query);

    return true;
}

void DFMSearcher::stop()
{
    if (engine && engine->status() == SearchStatus::Searching) {
        fmInfo() << "Stopping search for:" << keyword;
        engine->cancel();
    }
}

bool DFMSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

DFMSearchResultMap DFMSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    DFMSearchResultMap result = std::move(allResults);
    allResults.clear();
    return result;
}

SearchType DFMSearcher::getSearchType() const
{
    return engine ? engine->searchType() : SearchType::FileName;
}

void DFMSearcher::processSearchResult(const SearchResult &result)
{
    QUrl url = QUrl::fromLocalFile(result.path());

    // 创建统一的搜索结果数据结构
    DFMSearchResult searchResult(url);

    // 根据搜索类型设置不同的字段
    if (engine->searchType() == SearchType::Content) {
        ContentResultAPI contentResult(const_cast<SearchResult &>(result));
        searchResult.setHighlightedContent(contentResult.highlightedContent());
        searchResult.setIsContentMatch(true);
        searchResult.setMatchScore(1.0);   // 内容匹配优先级更高
    } else {
        // 文件名搜索不包含高亮内容
        searchResult.setIsContentMatch(false);
        searchResult.setMatchScore(0.5);   // 文件名匹配优先级较低
    }

    // 安全地存储结果
    QMutexLocker lk(&mutex);
    allResults.insert(url, searchResult);
}

void DFMSearcher::onSearchStarted()
{
    fmInfo() << "Search started for:" << keyword;
}

void DFMSearcher::onSearchFinished(const QList<SearchResult> &results)
{
    // 处理最后一批结果
    for (const auto &result : results) {
        processSearchResult(result);
    }

    // 最后一次通知有新结果
    if (!allResults.isEmpty())
        emit unearthed(this);

    // 搜索完成
    emit finished();
}

void DFMSearcher::onSearchCancelled()
{
    auto type = getSearchType();
    fmInfo() << "Search cancelled for:" << keyword << (type == SearchType::FileName ? "File name" : "Content");
    emit finished();
}

void DFMSearcher::onSearchError(const SearchError &error)
{
    fmWarning() << "Search error:" << error.message() << "for query:" << keyword;
    emit finished();
}
