// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "querystrategies.h"

#include <QRegularExpression>

DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

namespace {

const QRegularExpression kWhitespacePattern { QStringLiteral("\\s") };
const QRegularExpression kWhitespaceDelimiter { QStringLiteral("\\s+") };

bool isBooleanKeyword(const QString &keyword)
{
    return keyword.contains(kWhitespacePattern);
}

bool isWildcardKeyword(const QString &keyword, SearchType searchType)
{
    if (searchType != SearchType::FileName)
        return false;

    return keyword.contains('*') || keyword.contains('?');
}

}   // namespace

QuerySyntaxType QuerySyntaxUtils::detect(const QString &keyword, SearchType searchType)
{
    // Keep this order aligned with QueryTypeSelector so every caller interprets
    // the same keyword with the same explicit-search precedence.
    if (isBooleanKeyword(keyword))
        return QuerySyntaxType::Boolean;

    if (isWildcardKeyword(keyword, searchType))
        return QuerySyntaxType::Wildcard;

    return QuerySyntaxType::Simple;
}

// ============== SimpleQueryStrategy 实现 ==============

SearchQuery SimpleQueryStrategy::createQuery(const QString &keyword) const
{
    fmDebug() << "Create SimpleQueryStrategy for search" << keyword;
    return SearchFactory::createQuery(keyword, SearchQuery::Type::Simple);
}

bool SimpleQueryStrategy::canHandle(const QString &keyword, SearchType searchType) const
{
    Q_UNUSED(keyword)
    Q_UNUSED(searchType)
    // 简单查询是默认的回退策略，总是能处理
    return true;
}

// ============== WildcardQueryStrategy 实现 ==============

SearchQuery WildcardQueryStrategy::createQuery(const QString &keyword) const
{
    fmDebug() << "Create WildcardQueryStrategy for search" << keyword;
    return SearchFactory::createQuery(keyword, SearchQuery::Type::Wildcard);
}

bool WildcardQueryStrategy::canHandle(const QString &keyword, SearchType searchType) const
{
    return QuerySyntaxUtils::detect(keyword, searchType) == QuerySyntaxType::Wildcard;
}

// ============== BooleanQueryStrategy 实现 ==============

SearchQuery BooleanQueryStrategy::createQuery(const QString &keyword) const
{
    fmDebug() << "Create BooleanQueryStrategy for search" << keyword;
    QStringList keywords = keyword.split(kWhitespaceDelimiter, Qt::SkipEmptyParts);
    SearchQuery query = SearchFactory::createQuery(keywords, SearchQuery::Type::Boolean);
    // 设置布尔操作符为 AND
    query.setBooleanOperator(SearchQuery::BooleanOperator::AND);
    return query;
}

bool BooleanQueryStrategy::canHandle(const QString &keyword, SearchType searchType) const
{
    Q_UNUSED(searchType)
    return QuerySyntaxUtils::detect(keyword, SearchType::FileName) == QuerySyntaxType::Boolean;
}

// ============== QueryTypeSelector 实现 ==============

QueryTypeSelector::QueryTypeSelector()
{
    // 按优先级顺序添加策略
    // 1. 布尔查询（优先级最高，处理多关键词）
    // 2. 通配符查询（处理包含通配符的单关键词）
    // 3. 简单查询（默认回退策略）
    strategies.append(QSharedPointer<QueryTypeStrategy>(new BooleanQueryStrategy()));
    strategies.append(QSharedPointer<QueryTypeStrategy>(new WildcardQueryStrategy()));
    strategies.append(QSharedPointer<QueryTypeStrategy>(new SimpleQueryStrategy()));
}

SearchQuery QueryTypeSelector::createQuery(const QString &keyword, SearchType searchType) const
{
    for (const auto &strategy : strategies) {
        if (strategy->canHandle(keyword, searchType)) {
            return strategy->createQuery(keyword);
        }
    }

    // 理论上不应该到达这里，因为SimpleQueryStrategy总是返回true
    return SearchFactory::createQuery(keyword, SearchQuery::Type::Simple);
}

const QList<QSharedPointer<QueryTypeStrategy>> &QueryTypeSelector::getStrategies() const
{
    return strategies;
}
