// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsearcher.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>

#include <dfm-search/searchfactory.h>

#include <QDebug>

static constexpr int kEmitInterval = 50;   // Notification time interval (ms)

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

    notifyTimer.start();

    // Set search options
    SearchOptions options;
    options.setSearchMethod(SearchMethod::Indexed);
    options.setSearchPath(path);
    options.setCaseSensitive(false);
    options.setIncludeHidden(Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool());
    engine->setSearchOptions(options);

    // Create and execute search query
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
    
    // Create unified search result data structure
    DFMSearchResult searchResult(url);
    
    // If it's content search, parse highlighted content
    if (engine->searchType() == SearchType::Content) {
        ContentResultAPI contentResult(const_cast<SearchResult &>(result));
        searchResult.setHighlightedContent(contentResult.highlightedContent());
        searchResult.setIsContentMatch(true);
        searchResult.setMatchScore(1.0); // Content match has higher priority
    } else {
        // Filename search doesn't include highlighted content
        searchResult.setIsContentMatch(false);
        searchResult.setMatchScore(0.5); // Filename match has lower priority
    }

    // Results obtained by DFMSearcher can be considered unique within this instance
    QMutexLocker lk(&mutex);
    allResults.insert(url, searchResult);
}

void DFMSearcher::onResultFound(const SearchResult &result)
{
    processSearchResult(result);

    // Send results every 50ms
    if (notifyTimer.elapsed() - lastEmit > kEmitInterval) {
        lastEmit = notifyTimer.elapsed();
        emit unearthed(this);
    }
}

void DFMSearcher::onSearchFinished(const QList<SearchResult> &results)
{
    fmInfo() << "Search finished, found" << results.size() << "results";
    if (!engine->searchOptions().resultFoundEnabled()) {
        for (const auto &result : results) {
            processSearchResult(result);
        }
    }

    fmInfo() << "Search finished, result processed";
    if (engine->status() == SearchStatus::Finished && hasItem())
        emit unearthed(this);

    fmInfo() << "Search finished, result pushed";
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
