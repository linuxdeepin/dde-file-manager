// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticadapter.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

SemanticAdapter::SemanticAdapter(const QUrl &url, const QString &keyword, QObject *parent)
    : AbstractSearcher(url, keyword, parent)
{
    semantic = new DFMSEARCH::SemanticSearcher(this);
    connect(semantic, &DFMSEARCH::SemanticSearcher::resultsFound,
            this, &SemanticAdapter::onResultsFound);
    connect(semantic, &DFMSEARCH::SemanticSearcher::searchFinished,
            this, &SemanticAdapter::onSearchFinished);
    connect(semantic, &DFMSEARCH::SemanticSearcher::searchCancelled,
            this, &SemanticAdapter::onSearchCancelled);
    connect(semantic, &DFMSEARCH::SemanticSearcher::errorOccurred,
            this, &SemanticAdapter::onErrorOccurred);
}

SemanticAdapter::~SemanticAdapter()
{
}

bool SemanticAdapter::search()
{
    fmInfo() << "Starting semantic search for keyword:" << keyword << "in URL:" << searchUrl.toString();

    if (keyword.isEmpty()) {
        fmWarning() << "Semantic search keyword is empty, abort";
        emit finished();
        return true;
    }

    // 把 searchUrl 转成本地路径，作为 searchDirectories 显式传入，
    // 避免语义搜索回退到 home 目录导致跨目录扫描
    const QString &path = UrlRoute::urlToPath(searchUrl);
    const QString transformedPath = FileUtils::bindPathTransform(path, false);
    semantic->search(keyword, QStringList { transformedPath });
    return true;
}

void SemanticAdapter::stop()
{
    if (semantic) {
        fmInfo() << "Cancelling semantic search for:" << keyword;
        semantic->cancel();
    }
}

bool SemanticAdapter::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

DFMSearchResultMap SemanticAdapter::takeAll()
{
    QMutexLocker lk(&mutex);
    DFMSearchResultMap result = std::move(allResults);
    allResults.clear();
    return result;
}

void SemanticAdapter::processResult(const DFMSEARCH::SearchResult &result)
{
    const QUrl url = QUrl::fromLocalFile(result.path());
    if (url.isEmpty())
        return;

    DFMSearchResult sr(url);
    sr.setKeyword(keyword);
    sr.setSearchType(DFMSEARCH::SearchType::Semantic);
    sr.setMatchScore(kSemanticMatchScore);

    QMutexLocker lk(&mutex);
    allResults.insert(url, sr);
}

void SemanticAdapter::onResultsFound(const DFMSEARCH::SearchResultList &results)
{
    for (const auto &r : results)
        processResult(r);

    {
        QMutexLocker lk(&mutex);
        if (!allResults.isEmpty()) {
            lk.unlock();
            emit unearthed(this);
        }
    }
}

void SemanticAdapter::onSearchFinished(const DFMSEARCH::SearchResultList &results)
{
    fmInfo() << "Semantic search finished for keyword:" << keyword << "results count:" << results.size();

    for (const auto &r : results)
        processResult(r);

    {
        QMutexLocker lk(&mutex);
        if (!allResults.isEmpty()) {
            lk.unlock();
            emit unearthed(this);
        }
    }
    emit finished();
}

void SemanticAdapter::onSearchCancelled()
{
    fmInfo() << "Semantic search cancelled for:" << keyword;
    emit finished();
}

void SemanticAdapter::onErrorOccurred(const DFMSEARCH::SearchError &error)
{
    fmWarning() << "Semantic search error for keyword:" << keyword
                << "message:" << error.message();
    emit finished();
}
