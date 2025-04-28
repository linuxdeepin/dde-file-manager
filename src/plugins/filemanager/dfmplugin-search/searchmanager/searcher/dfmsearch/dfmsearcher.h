// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSEARCHER_H
#define DFMSEARCHER_H

#include "searchmanager/searcher/abstractsearcher.h"
#include <dfm-search/searchengine.h>
#include <dfm-search/searchquery.h>
#include <dfm-search/searchoptions.h>
#include <dfm-search/contentsearchapi.h>

#include <QMutex>
#include <QUrl>

DPSEARCH_BEGIN_NAMESPACE

class DFMSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;

public:
    explicit DFMSearcher(const QUrl &url, const QString &keyword, QObject *parent, DFMSEARCH::SearchType type);
    ~DFMSearcher() override;

    static bool supportUrl(const QUrl &url);
    static QString realSearchPath(const QUrl &url);

    bool search() override;
    void stop() override;
    bool hasItem() const override;
    DFMSearchResultMap takeAll() override;

private:
    DFMSEARCH::SearchQuery createSearchQuery() const;
    void processSearchResult(const DFMSEARCH::SearchResult &result);
    DFMSEARCH::SearchType getSearchType() const;
    DFMSEARCH::SearchMethod getSearchMethod(const QString &path) const;
    void handleRemainingResults(const QList<DFMSEARCH::SearchResult> &results);
    bool isEngineReady() const;
    bool isValidSearchParameters() const;
    DFMSEARCH::SearchOptions configureSearchOptions(const QString &transformedPath) const;
    bool validateSearchType(const QString &transformedPath, DFMSEARCH::SearchOptions &options);
    void executeSearch();

private slots:
    void onSearchStarted();
    void onSearchFinished(const QList<DFMSEARCH::SearchResult> &results);
    void onSearchCancelled();
    void onSearchError(const DFMSEARCH::SearchError &error);

private:
    DFMSEARCH::SearchEngine *engine { nullptr };
    DFMSearchResultMap allResults;
    mutable QMutex mutex;

    // 批处理大小，每找到这么多个结果就通知一次
    static constexpr int kBatchSize = 10;
};

DPSEARCH_END_NAMESPACE

#endif   // DFMSEARCHER_H
