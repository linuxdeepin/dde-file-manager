// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keywordextractor.h"

#include <dfm-base/base/application/application.h>

#include <QUrlQuery>
#include <QUrl>
#include <QDebug>
#include <algorithm>

using namespace dfmplugin_workspace;

// ============== SimpleKeywordStrategy 实现 ==============

QStringList SimpleKeywordStrategy::extractKeywords(const QString &keyword) const
{
    if (keyword.isEmpty()) {
        fmDebug() << "SimpleKeywordStrategy: Empty keyword provided";
        return {};
    }

    fmDebug() << "SimpleKeywordStrategy: Extracting simple keyword:" << keyword;
    // 简单策略：直接返回原始关键字
    return { keyword };
}

bool SimpleKeywordStrategy::canHandle(const QString &keyword) const
{
    Q_UNUSED(keyword)
    // 简单策略是默认的回退策略，总是能处理
    fmDebug() << "SimpleKeywordStrategy: Can handle any keyword (fallback strategy)";
    return true;
}

// ============== BooleanKeywordStrategy 实现 ==============

QStringList BooleanKeywordStrategy::extractKeywords(const QString &keyword) const
{
    if (keyword.isEmpty()) {
        fmDebug() << "BooleanKeywordStrategy: Empty keyword provided";
        return {};
    }

    fmDebug() << "BooleanKeywordStrategy: Extracting boolean keywords from:" << keyword;

    // 按空白字符分割关键字
    QStringList keywords = keyword.split(whitespaceDelimiter, Qt::SkipEmptyParts);

    // 过滤掉空字符串和过短的关键字
    keywords.erase(std::remove_if(keywords.begin(), keywords.end(),
                                  [](const QString &str) {
                                      return str.trimmed().length() < 1;
                                  }),
                   keywords.end());

    return keywords;
}

bool BooleanKeywordStrategy::canHandle(const QString &keyword) const
{
    // 检查是否包含空白字符
    return keyword.contains(whitespacePattern);
}

// ============== WildcardKeywordStrategy 实现 ==============

QStringList WildcardKeywordStrategy::extractKeywords(const QString &keyword) const
{
    if (keyword.isEmpty()) {
        fmDebug() << "WildcardKeywordStrategy: Empty keyword provided";
        return {};
    }

    return extractFromWildcardPattern(keyword);
}

bool WildcardKeywordStrategy::canHandle(const QString &keyword) const
{
    // 检查是否包含通配符字符
    return keyword.contains('*') || keyword.contains('?');
}

QStringList WildcardKeywordStrategy::extractFromWildcardPattern(const QString &pattern) const
{
    fmDebug() << "WildcardKeywordStrategy: Processing wildcard pattern:" << pattern;

    QStringList keywords;

    // 首先添加原始模式本身作为第一个关键字
    keywords.append(pattern);

    // 将通配符替换为空格，然后分割
    QString modifiedPattern = pattern;
    modifiedPattern.replace('*', ' ').replace('?', ' ');

    // 按空格分割并过滤
    QStringList segments = modifiedPattern.split(' ', Qt::SkipEmptyParts);

    for (const QString &segment : segments) {
        QString trimmedSegment = segment.trimmed();
        // 保留所有非空的有意义片段（长度 >= 1）
        if (!trimmedSegment.isEmpty()) {
            keywords.append(trimmedSegment);
        }
    }

    // 去重，保持顺序（原始模式总是第一个）
    QStringList uniqueKeywords;
    for (const QString &keyword : keywords) {
        if (!uniqueKeywords.contains(keyword)) {
            uniqueKeywords.append(keyword);
        }
    }

    fmDebug() << "WildcardKeywordStrategy: Final unique keywords - count:" << uniqueKeywords.size() << "keywords:" << uniqueKeywords;
    return uniqueKeywords;
}

// ============== KeywordExtractor 实现 ==============

KeywordExtractor::KeywordExtractor()
{
    fmDebug() << "KeywordExtractor: Initializing with default strategies";

    // 按优先级顺序注册策略
    registerStrategy(QSharedPointer<KeywordExtractionStrategy>(new BooleanKeywordStrategy()));
    registerStrategy(QSharedPointer<KeywordExtractionStrategy>(new WildcardKeywordStrategy()));
    registerStrategy(QSharedPointer<KeywordExtractionStrategy>(new SimpleKeywordStrategy()));

    fmDebug() << "KeywordExtractor: Initialized with" << strategies.size() << "strategies";
}

QStringList KeywordExtractor::extractFromUrl(const QUrl &url) const
{
    QUrlQuery query(url.query());
    if (!query.hasQueryItem("keyword")) {
        fmDebug() << "KeywordExtractor: No keyword query item found in URL";
        return {};
    }

    QString keywordValue = query.queryItemValue("keyword");
    // 先对URL编码的字符进行解码，确保能处理所有空白字符
    keywordValue = QUrl::fromPercentEncoding(keywordValue.toUtf8());
    fmDebug() << "KeywordExtractor: Decoded keyword value:" << keywordValue;

    return extractFromKeyword(keywordValue);
}

QStringList KeywordExtractor::extractFromKeyword(const QString &keyword) const
{
    if (keyword.isEmpty()) {
        fmDebug() << "KeywordExtractor: Empty keyword provided";
        return {};
    }

    fmDebug() << "KeywordExtractor: Extracting keywords from:" << keyword << "using" << strategies.size() << "strategies";

    // 按优先级顺序尝试每个策略
    for (const auto &strategy : strategies) {
        if (strategy->canHandle(keyword)) {
            QStringList result = strategy->extractKeywords(keyword);
            if (!result.isEmpty()) {
                return result;
            }
        }
    }

    // 理论上不应该到达这里，因为SimpleKeywordStrategy总是返回true
    fmWarning() << "KeywordExtractor: No strategy could handle keyword:" << keyword;
    return {};
}

void KeywordExtractor::registerStrategy(QSharedPointer<KeywordExtractionStrategy> strategy)
{
    if (!strategy) {
        fmWarning() << "KeywordExtractor: Attempted to register null strategy";
        return;
    }

    fmDebug() << "KeywordExtractor: Registering strategy with priority:" << strategy->priority();
    strategies.append(strategy);
    sortStrategiesByPriority();
}

const QList<QSharedPointer<KeywordExtractionStrategy>> &KeywordExtractor::getStrategies() const
{
    fmDebug() << "KeywordExtractor: Returning" << strategies.size() << "strategies";
    return strategies;
}

void KeywordExtractor::sortStrategiesByPriority()
{
    fmDebug() << "KeywordExtractor: Sorting strategies by priority";
    std::sort(strategies.begin(), strategies.end(),
              [](const QSharedPointer<KeywordExtractionStrategy> &a,
                 const QSharedPointer<KeywordExtractionStrategy> &b) {
                  return a->priority() < b->priority();
              });
}

// ============== KeywordExtractorManager 实现 ==============

KeywordExtractorManager &KeywordExtractorManager::instance()
{
    static KeywordExtractorManager instance;
    fmDebug() << "KeywordExtractorManager: Returning singleton instance";
    return instance;
}

KeywordExtractor &KeywordExtractorManager::extractor()
{
    fmDebug() << "KeywordExtractorManager: Returning extractor reference";
    return keywordExtractor;
}
