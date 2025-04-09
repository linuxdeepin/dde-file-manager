// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
    QList<QUrl> takeAll() override;
    QMap<QUrl, ContentSearchResult> takeContentResults() override;

private:
    DFMSEARCH::SearchQuery createSearchQuery() const;
    void processSearchResult(const DFMSEARCH::SearchResult &result);

private slots:
    void onSearchStarted();
    void onResultFound(const DFMSEARCH::SearchResult &result);
    void onSearchFinished(const QList<DFMSEARCH::SearchResult> &results);
    void onSearchCancelled();
    void onSearchError(const DFMSEARCH::SearchError &error);

private:
    DFMSEARCH::SearchEngine *engine { nullptr };
    QMap<QUrl, ContentSearchResult> allResults;
    mutable QMutex mutex;
    QElapsedTimer notifyTimer;
    int lastEmit { 0 };
};

DPSEARCH_END_NAMESPACE

#endif   // DFMSEARCHER_H 
