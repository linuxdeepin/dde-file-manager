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
    connect(engine, &SearchEngine::resultsFound, this, [this](const DFMSEARCH::SearchResultList &results) {
        fmDebug() << "Real time found " << results.size() << "results";
        handleRemainingResults(results);
    });
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

QString DFMSearcher::realSearchPath(const QUrl &url)
{
    const QString &path = UrlRoute::urlToPath(url);
    return FileUtils::bindPathTransform(path, false);
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
    if (!isEngineReady() || !isValidSearchParameters()) {
        return false;
    }

    QString transformedPath = realSearchPath(searchUrl);
    SearchOptions options = configureSearchOptions(transformedPath);

    if (!validateSearchType(transformedPath, options)) {
        emit finished();
        return true;   // Early exit if validation fails
    }

    engine->setSearchOptions(options);
    executeSearch();

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

bool DFMSearcher::isEngineReady() const
{
    return engine && engine->status() == SearchStatus::Ready;
}

bool DFMSearcher::isValidSearchParameters() const
{
    const QString &path = UrlRoute::urlToPath(searchUrl);
    return !path.isEmpty() && !keyword.isEmpty();
}

bool DFMSearcher::validateSearchType(const QString &transformedPath, SearchOptions &options)
{
    if (engine->searchType() == SearchType::Content) {
        if (!DFMSEARCH::Global::isPathInFileNameIndexDirectory(transformedPath)) {
            fmInfo() << "Full-text search is currently only supported for Indexed, current path not indexed: " << transformedPath;
            return false;
        } else {
            ContentOptionsAPI contentAPI(options);
            contentAPI.setMaxPreviewLength(200);
        }
    }
    return true;
}

void DFMSearcher::executeSearch()
{
    SearchQuery query = createSearchQuery();
    engine->search(query);
}

SearchOptions DFMSearcher::configureSearchOptions(const QString &transformedPath) const
{
    SearchOptions options;
    options.setSearchMethod(getSearchMethod(transformedPath));
    options.setSearchPath(transformedPath);
    options.setCaseSensitive(false);
    options.setIncludeHidden(Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool());

    if (options.method() == SearchMethod::Realtime) {
        options.setResultFoundEnabled(true);
        auto excludedPaths = DFMSEARCH::Global::defaultIndexedDirectory();
        QStringList transPaths;

        std::transform(excludedPaths.begin(), excludedPaths.end(), std::back_inserter(transPaths),
                       [](const QString &path) {
                           return FileUtils::bindPathTransform(path, true);
                       });

        transPaths.erase(std::remove_if(transPaths.begin(), transPaths.end(),
                                        [&excludedPaths](const QString &transPath) {
                                            return excludedPaths.contains(transPath);
                                        }),
                         transPaths.end());

        if (!transPaths.isEmpty()) {
            excludedPaths.append(transPaths);
        }

        options.setSearchExcludedPaths(excludedPaths);
    }

    return options;
}

void DFMSearcher::handleRemainingResults(const QList<SearchResult> &results)
{
    for (const auto &result : results) {
        processSearchResult(result);
    }

    if (!allResults.isEmpty()) {
        emit unearthed(this);
    }
}

SearchMethod DFMSearcher::getSearchMethod(const QString &path) const
{
    if (engine->searchType() == SearchType::FileName
        && !DFMSEARCH::Global::isPathInFileNameIndexDirectory(path)) {
        fmInfo() << "Use realtime methdo to: " << path;
        return SearchMethod::Realtime;
    }

    return SearchMethod::Indexed;
}

void DFMSearcher::onSearchStarted()
{
    fmInfo() << "Search started for:" << keyword;
}

void DFMSearcher::onSearchFinished(const QList<SearchResult> &results)
{
    fmDebug() << engine->searchType() << "search finished, results count:" << results.size();
    if (!engine->searchOptions().resultFoundEnabled()) {
        handleRemainingResults(results);
    }
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
