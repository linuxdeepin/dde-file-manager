// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULT_DEFINE_H
#define SEARCHRESULT_DEFINE_H

#include "dfmplugin_search_global.h"

#include <dfm-search/searchquery.h>

#include <QUrl>
#include <QMap>
#include <QSharedData>

DPSEARCH_BEGIN_NAMESPACE

// 统一的搜索结果数据结构，使用隐式共享提高性能
class DFMSearchResultData : public QSharedData
{
public:
    DFMSearchResultData() = default;
    DFMSearchResultData(const QUrl &u)
        : url(u) { }

    QUrl url {};   // 搜索结果URL
    QString keyword {};   // 搜索关键词（供延迟加载 highlight 使用）
    DFMSEARCH::SearchType searchType { DFMSEARCH::SearchType::FileName };   // 搜索类型（供延迟加载 highlight 使用）
    double matchScore { 0.0 };   // 匹配分数，用于排序
};

// 使用隐式共享的搜索结果，提高拷贝和传递效率
class DFMSearchResult
{
public:
    DFMSearchResult()
        : d(new DFMSearchResultData()) { }
    DFMSearchResult(const QUrl &url)
        : d(new DFMSearchResultData(url)) { }

    inline QUrl url() const { return d->url; }
    inline void setUrl(const QUrl &url) { d->url = url; }

    inline QString keyword() const { return d->keyword; }
    inline void setKeyword(const QString &kw) { d->keyword = kw; }

    inline DFMSEARCH::SearchType searchType() const { return d->searchType; }
    inline void setSearchType(DFMSEARCH::SearchType type) { d->searchType = type; }

    inline double matchScore() const { return d->matchScore; }
    inline void setMatchScore(double score) { d->matchScore = score; }

private:
    QSharedDataPointer<DFMSearchResultData> d;
};

// 使用QMap的优点：1.按URL自动排序 2.自动去重 3.提供高效查找
typedef QMap<QUrl, DFMSearchResult> DFMSearchResultMap;

DPSEARCH_END_NAMESPACE

#endif   // SEARCHRESULT_DEFINE_H
