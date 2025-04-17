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
    // connect(engine, &SearchEngine::resultFound, this, &DFMSearcher::onResultFound);
    connect(engine, &SearchEngine::searchFinished, this, &DFMSearcher::onSearchFinished);
    connect(engine, &SearchEngine::searchCancelled, this, &DFMSearcher::onSearchCancelled);
    connect(engine, &SearchEngine::errorOccurred, this, &DFMSearcher::onSearchError);
}

DFMSearcher::~DFMSearcher()
{
    // Cancel and wait for any background processing to finish before destruction
    if (processingFuture.isRunning()) {
        fmInfo() << "Canceling background result processing during destruction";
        processingFuture.cancel();
        processingFuture.waitForFinished();
    }
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

    notifyTimer.start();
    lastEmit.storeRelaxed(0);
    resultCount.storeRelaxed(0);

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

void DFMSearcher::stop()
{
    if (engine && engine->status() == SearchStatus::Searching) {
        fmInfo() << "Stopping search for:" << keyword;
        engine->cancel();
    }
    
    // Cancel and wait for any background processing to finish
    if (processingFuture.isRunning()) {
        fmInfo() << "Canceling background result processing";
        processingFuture.cancel();
        processingFuture.waitForFinished();
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
    resultCount.storeRelaxed(0);
    return result;
}

SearchType DFMSearcher::getSearchType() const
{
    return engine ? engine->searchType() : SearchType::FileName;
}

void DFMSearcher::checkNotifyThreshold()
{
    // Send notification signal if we have reached the batch size or time interval
    bool shouldNotify = false;
    
    // Check batch size threshold
    if (resultCount.loadRelaxed() >= kBatchSize) {
        shouldNotify = true;
    } else {
        // Check time interval threshold (if we have any results to send)
        if (resultCount.loadRelaxed() > 0 && 
            notifyTimer.elapsed() - lastEmit.loadRelaxed() > kEmitInterval) {
            shouldNotify = true;
        }
    }
    
    if (shouldNotify) {
        lastEmit.storeRelaxed(notifyTimer.elapsed());
        resultCount.storeRelaxed(0);
        emit unearthed(this);
    }
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

    // Store the result in a thread-safe way
    QMutexLocker lk(&mutex);
    allResults.insert(url, searchResult);
    
    // Increment result count for batch processing
    resultCount.fetchAndAddRelaxed(1);
}

// void DFMSearcher::onResultFound(const SearchResult &result)
// {
//     processSearchResult(result);
//     checkNotifyThreshold();
// }

void DFMSearcher::processResultsAsync(const QList<SearchResult> &results)
{
    // Process all results in a background thread
    for (const auto &result : results) {
        processSearchResult(result);
    }
    
    // Notify the UI thread about the results
    QMetaObject::invokeMethod(this, [this]() {
        if (resultCount.loadRelaxed() > 0) {
            emit unearthed(this);
            resultCount.storeRelaxed(0);
        }
        emit finished();
    }, Qt::QueuedConnection);
    
    fmInfo() << "Finished processing" << results.size() << "search results in background thread";
}

void DFMSearcher::onSearchFinished(const QList<SearchResult> &results)
{
    fmInfo() << "Search finished, found" << results.size() << "results";
    
    if (!engine->searchOptions().resultFoundEnabled()) {
        if (results.size() > 50) {  // Process in background thread if we have many results
            fmInfo() << "Processing large result set asynchronously";
            processingFuture = QtConcurrent::run([this, results]() {
                processResultsAsync(results);
            });
        } else {
            // Process directly for small result sets
            for (const auto &result : results) {
                processSearchResult(result);
            }
            
            // Ensure we notify about any remaining results
            if (resultCount.loadRelaxed() > 0) {
                emit unearthed(this);
            }
            
            fmInfo() << "Search finished, result pushed";
            emit finished();
        }
    } else {
        // In this case, results have already been processed via onResultFound
        // Just make sure we emit any remaining results
        if (resultCount.loadRelaxed() > 0) {
            emit unearthed(this);
        }
        
        fmInfo() << "Search finished, result pushed";
        emit finished();
    }
}

void DFMSearcher::onSearchCancelled()
{
    fmInfo() << "Search cancelled";
    
    // Send any remaining results
    if (resultCount.loadRelaxed() > 0) {
        emit unearthed(this);
    }
    
    emit finished();
}

void DFMSearcher::onSearchError(const SearchError &error)
{
    fmWarning() << "Search error:"
                << "Code:" << error.code().value()
                << "Category:" << error.code().category().name()
                << "Name:" << error.name()
                << "Message:" << error.message();
                
    // Send any results we have so far
    if (resultCount.loadRelaxed() > 0) {
        emit unearthed(this);
    }
    
    emit finished();
} 
