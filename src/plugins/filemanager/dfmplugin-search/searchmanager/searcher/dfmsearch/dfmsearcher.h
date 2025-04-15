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
#include <QElapsedTimer>
#include <QMap>
#include <QAtomicInt>

DPSEARCH_BEGIN_NAMESPACE

class DFMSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;

public:
    explicit DFMSearcher(const QUrl &url, const QString &keyword, QObject *parent, DFMSEARCH::SearchType type);
    ~DFMSearcher() override;

    bool search() override;
    bool hasItem() const override;
    DFMSearchResultMap takeAll() override;

private:
    DFMSEARCH::SearchQuery createSearchQuery() const;
    void processSearchResult(const DFMSEARCH::SearchResult &result);
    DFMSEARCH::SearchType getSearchType() const;
    
    // Improves thread safety by using atomic value for notification tracking
    void checkNotifyThreshold();

private slots:
    void onSearchStarted();
    // void onResultFound(const DFMSEARCH::SearchResult &result);
    void onSearchFinished(const QList<DFMSEARCH::SearchResult> &results);
    void onSearchCancelled();
    void onSearchError(const DFMSEARCH::SearchError &error);

private:
    DFMSEARCH::SearchEngine *engine { nullptr };
    DFMSearchResultMap allResults;
    mutable QMutex mutex;
    QElapsedTimer notifyTimer;
    QAtomicInt lastEmit { 0 };
    QAtomicInt resultCount { 0 };
    static constexpr int kBatchSize = 10;    // Number of results to batch before notifying
    static constexpr int kEmitInterval = 50; // Milliseconds between notifications
};

DPSEARCH_END_NAMESPACE

#endif   // DFMSEARCHER_H 
