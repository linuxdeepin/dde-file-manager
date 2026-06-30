// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICADAPTER_H
#define SEMANTICADAPTER_H

#include "searchmanager/searcher/abstractsearcher.h"

#include <dfm-search/semanticsearcher.h>
#include <dfm-search/searchresult.h>
#include <dfm-search/searcherror.h>
#include <dfm-search/semantic_types.h>

#include <QMutex>

DPSEARCH_BEGIN_NAMESPACE

/**
 * @brief 把独立的 SemanticSearcher 适配到 AbstractSearcher 契约。
 *
 * SemanticSearcher 与基于 SearchFactory::createEngine 的体系完全独立：
 * 它不接受 SearchType，内部自行编排 FileName/Content/OCR 子引擎，对外
 * 只暴露 search(naturalLanguage, searchDirectories) 与 resultsFound/
 * searchFinished 信号。本适配器把这些信号翻译为 AbstractSearcher 的
 * unearthed/finished 信号，让 SimplifiedSearchWorker 无感知地接入语义搜索。
 *
 * matchScore 设为 0.25，低于 FileName(0.5) 和 Content/Ocr(1.0)，
 * SimplifiedSearchWorker::mergeResults 会按 matchScore 高者优先自动去重，
 * 因此语义匹配结果在精确匹配存在时会被丢弃。
 */
class SemanticAdapter : public AbstractSearcher
{
    Q_OBJECT
public:
    explicit SemanticAdapter(const QUrl &url, const QString &keyword, QObject *parent = nullptr);
    ~SemanticAdapter() override;

    bool search() override;
    void stop() override;
    bool hasItem() const override;
    DFMSearchResultMap takeAll() override;

private slots:
    void onIntentParsed(const DFMSEARCH::ParsedIntent &intent);
    void onResultsFound(const DFMSEARCH::SearchResultList &results);
    void onSearchFinished(const DFMSEARCH::SearchResultList &results);
    void onSearchCancelled();
    void onErrorOccurred(const DFMSEARCH::SearchError &error);

private:
    void processResult(const DFMSEARCH::SearchResult &result);

    DFMSEARCH::SemanticSearcher *semantic { nullptr };
    mutable QMutex mutex;
    DFMSearchResultMap allResults;

    // 由 intentParsed 信号缓存的关键词列表（已去语义噪声）。
    // join(' ') 后写入 DFMSearchResult.keyword，下游 requestHighlightContent
    // 取首个作锚点定位摘要，delegate 用全部作高亮匹配——复用单一数据源。
    QStringList cachedKeywords;
    mutable QMutex keywordMutex;

    // 语义匹配优先级低于精确匹配：FileName=0.5, Content/Ocr=1.0
    static constexpr double kSemanticMatchScore = 0.25;
};

DPSEARCH_END_NAMESPACE

#endif   // SEMANTICADAPTER_H
