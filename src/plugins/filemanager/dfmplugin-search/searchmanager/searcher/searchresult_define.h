// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULT_DEFINE_H
#define SEARCHRESULT_DEFINE_H

#include "dfmplugin_search_global.h"

#include <QUrl>
#include <QMap>
#include <QSharedData>

DPSEARCH_BEGIN_NAMESPACE

// 统一的搜索结果数据结构，使用隐式共享提高性能
class DFMSearchResultData : public QSharedData
{
public:
    DFMSearchResultData() = default;
    DFMSearchResultData(const QUrl &u, const QString &content = QString())
        : url(u), highlightedContent(content) {}

    QUrl url {};                    // 搜索结果URL
    QString highlightedContent {};  // 高亮的内容片段（全文搜索时使用）
    bool isContentMatch { false };  // 是否是内容匹配（区分文件名匹配和内容匹配）
    double matchScore { 0.0 };      // 匹配分数，用于排序
};

// 使用隐式共享的搜索结果，提高拷贝和传递效率
class DFMSearchResult
{
public:
    DFMSearchResult() : d(new DFMSearchResultData()) {}
    DFMSearchResult(const QUrl &url, const QString &content = QString())
        : d(new DFMSearchResultData(url, content)) {}
    
    inline QUrl url() const { return d->url; }
    inline void setUrl(const QUrl &url) { d->url = url; }
    
    inline QString highlightedContent() const { return d->highlightedContent; }
    inline void setHighlightedContent(const QString &content) { d->highlightedContent = content; }
    
    inline bool isContentMatch() const { return d->isContentMatch; }
    inline void setIsContentMatch(bool value) { d->isContentMatch = value; }

    inline double matchScore() const { return d->matchScore; }
    inline void setMatchScore(double score) { d->matchScore = score; }

private:
    QSharedDataPointer<DFMSearchResultData> d;
};

// 使用QMap的优点：1.按URL自动排序 2.自动去重 3.提供高效查找
typedef QMap<QUrl, DFMSearchResult> DFMSearchResultMap;

DPSEARCH_END_NAMESPACE

#endif // SEARCHRESULT_DEFINE_H
