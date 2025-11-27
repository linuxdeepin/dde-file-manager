// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsearcher.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceproxymanager.h>

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
        fmWarning() << "Failed to create search engine for type:" << static_cast<int>(type);
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
    return querySelector.createQuery(keyword, getSearchType());
}

bool DFMSearcher::search()
{
    fmInfo() << "Starting search process for keyword:" << keyword << "in URL:" << searchUrl.toString();

    if (!isEngineReady() || !isValidSearchParameters()) {
        fmWarning() << "Search engine not ready or invalid parameters - engine ready:" << isEngineReady() << "valid params:" << isValidSearchParameters();
        return false;
    }

    QString transformedPath = realSearchPath(searchUrl);
    fmDebug() << "Using transformed search path:" << transformedPath;

    SearchOptions options = configureSearchOptions(transformedPath);

    if (!validateSearchType(transformedPath, options)) {
        fmWarning() << "Search type validation failed for path:" << transformedPath;
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
        if (DFMSEARCH::Global::isFileNameIndexReadyForSearch()
            && !DFMSEARCH::Global::isPathInFileNameIndexDirectory(transformedPath)) {
            fmInfo() << "Full-text search is currently only supported for Indexed, current path not indexed: " << transformedPath;
            return false;
        } else {
            ContentOptionsAPI contentAPI(options);
            contentAPI.setMaxPreviewLength(200);
            contentAPI.setFilenameContentMixedAndSearchEnabled(true);
            fmDebug() << "Content search options configured - max preview length: 200, mixed search enabled";
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

    configureHiddenFilesOption(options, transformedPath);

    if (options.method() == SearchMethod::Realtime) {
        configureRealtimeSearchOptions(options, transformedPath);
    }

    return options;
}

void DFMSearcher::configureHiddenFilesOption(SearchOptions &options, const QString &transformedPath) const
{
    // Always include hidden files when searching in hidden directories
    bool includeHidden = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();
    bool inHiddenDir = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(transformedPath);
    if (inHiddenDir) {
        includeHidden = true;
    }
    options.setIncludeHidden(includeHidden);
    fmDebug() << "Hidden files option configured - include hidden:" << includeHidden;
}

void DFMSearcher::configureRealtimeSearchOptions(SearchOptions &options, const QString &transformedPath) const
{
    options.setResultFoundEnabled(true);

    // 判断是否需要排除索引路径
    if (shouldExcludeIndexedPaths(transformedPath)) {
        setExcludedPathsForRealtime(options);
    } else {
        fmDebug() << "No excluded paths needed for realtime search";
    }
}

bool DFMSearcher::shouldExcludeIndexedPaths(const QString &transformedPath) const
{
    // 在隐藏目录中搜索时，不排除索引路径
    if (DFMSEARCH::Global::isHiddenPathOrInHiddenDir(transformedPath)) {
        fmDebug() << "Not excluding indexed paths due to hidden directory search";
        return false;
    }

    // 当索引目录不可用时，不排除索引路径
    if (engine->searchType() == SearchType::FileName && !DFMSEARCH::Global::isFileNameIndexReadyForSearch()) {
        fmDebug() << "Not excluding indexed paths due to unavailable filename index directory";
        return false;
    }

    // 其他挂载点
    if (DevProxyMng->isFileOfExternalMounts(transformedPath)) {
        fmDebug() << "Not excluding indexed paths due to external mounts search";
        return false;
    }

    // 其他情况下，排除索引路径以避免重复搜索
    return true;
}

void DFMSearcher::setExcludedPathsForRealtime(SearchOptions &options) const
{
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
    // 不使用索引的情况：文件名搜索 且 (路径不在索引目录中 或 路径在隐藏目录中)
    if (engine->searchType() != SearchType::FileName)
        return SearchMethod::Indexed;

    // 对于文件名搜索，首先检查文件名索引目录是否可用
    if (!DFMSEARCH::Global::isFileNameIndexReadyForSearch()) {
        fmWarning() << "File name index directory is not available, falling back to realtime search for path:" << path;
        return SearchMethod::Realtime;
    }

    // 对于文件名搜索，检查是否需要使用实时搜索
    const bool inIndexDir = DFMSEARCH::Global::isPathInFileNameIndexDirectory(path);
    const bool inHiddenDir = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(path);

    if (!inIndexDir || inHiddenDir) {
        fmInfo() << "Use realtime method to: " << path << "- in index dir:" << inIndexDir << "in hidden dir:" << inHiddenDir;
        return SearchMethod::Realtime;
    }

    // 一个文件即使在anything的索引挂载点下，但是用户依然可能继续
    // 手动挂载其他文件系统，这种情况下anything并不会生成索引，
    // 因此需要切换搜索方法
    if (DevProxyMng->isFileOfExternalMounts(path)) {
        fmInfo() << "Use reltime method to: " << path << " - is external mount";
        return SearchMethod::Realtime;
    }

    fmDebug() << "Using indexed method for filename search";
    return SearchMethod::Indexed;
}

void DFMSearcher::onSearchStarted()
{
    fmInfo() << "Search started for:" << keyword << "search type:" << static_cast<int>(getSearchType());
}

void DFMSearcher::onSearchFinished(const QList<SearchResult> &results)
{
    fmInfo() << "Search finished for keyword:" << keyword << "search type:" << static_cast<int>(engine->searchType()) << "results count:" << results.size();

    if (!engine->searchOptions().resultFoundEnabled()) {
        handleRemainingResults(results);
    }
    // 搜索完成
    emit finished();
}

void DFMSearcher::onSearchCancelled()
{
    auto type = getSearchType();
    fmInfo() << "Search cancelled for:" << keyword << "type:" << (type == SearchType::FileName ? "File name" : "Content");
    emit finished();
}

void DFMSearcher::onSearchError(const SearchError &error)
{
    fmWarning() << "Search error occurred - message:" << error.message() << "query:" << keyword;
    emit finished();
}
