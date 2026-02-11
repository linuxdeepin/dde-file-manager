// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERYSTRATEGIES_H
#define QUERYSTRATEGIES_H

#include "dfmplugin_search_global.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/searchquery.h>
#include <dfm-search/dsearch_global.h>

#include <QString>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QList>

DPSEARCH_BEGIN_NAMESPACE

/**
 * @brief 查询类型选择策略的抽象基类
 *
 * 使用策略模式来处理不同类型的搜索查询创建，
 * 符合开闭原则，便于扩展新的查询类型。
 */
class QueryTypeStrategy
{
public:
    virtual ~QueryTypeStrategy() = default;

    /**
     * @brief 创建搜索查询
     * @param keyword 搜索关键词
     * @return 创建的搜索查询对象
     */
    virtual DFMSEARCH::SearchQuery createQuery(const QString &keyword) const = 0;

    /**
     * @brief 判断是否能处理指定的关键词和搜索类型
     * @param keyword 搜索关键词
     * @param searchType 搜索类型
     * @return 如果能处理返回true，否则返回false
     */
    virtual bool canHandle(const QString &keyword, DFMSEARCH::SearchType searchType) const = 0;
};

/**
 * @brief 简单查询策略
 *
 * 处理不包含空白字符和通配符的简单关键词搜索。
 * 这是默认的回退策略。
 */
class SimpleQueryStrategy : public QueryTypeStrategy
{
public:
    DFMSEARCH::SearchQuery createQuery(const QString &keyword) const override;
    bool canHandle(const QString &keyword, DFMSEARCH::SearchType searchType) const override;
};

/**
 * @brief 通配符查询策略
 *
 * 处理包含通配符（* 和 ?）的搜索查询。
 * 仅支持文件名搜索，不支持内容搜索。
 */
class WildcardQueryStrategy : public QueryTypeStrategy
{
public:
    DFMSEARCH::SearchQuery createQuery(const QString &keyword) const override;
    bool canHandle(const QString &keyword, DFMSEARCH::SearchType searchType) const override;
};

/**
 * @brief 布尔查询策略
 *
 * 处理包含空白字符的多关键词搜索，使用AND操作符连接。
 * 不支持包含通配符的关键词。
 */
class BooleanQueryStrategy : public QueryTypeStrategy
{
public:
    DFMSEARCH::SearchQuery createQuery(const QString &keyword) const override;
    bool canHandle(const QString &keyword, DFMSEARCH::SearchType searchType) const override;

private:
    // 成员变量用于正则表达式匹配
    const QRegularExpression whitespacePattern { "\\s" };
    const QRegularExpression whitespaceDelimiter { "\\s+" };
};

/**
 * @brief 查询类型选择器
 *
 * 负责根据关键词和搜索类型选择合适的策略来创建搜索查询。
 * 策略按优先级顺序进行检查。
 */
class QueryTypeSelector
{
public:
    QueryTypeSelector();

    /**
     * @brief 创建搜索查询
     * @param keyword 搜索关键词
     * @param searchType 搜索类型
     * @return 创建的搜索查询对象
     */
    DFMSEARCH::SearchQuery createQuery(const QString &keyword, DFMSEARCH::SearchType searchType) const;

    /**
     * @brief 获取策略列表（主要用于测试）
     * @return 策略列表
     */
    const QList<QSharedPointer<QueryTypeStrategy>> &getStrategies() const;

private:
    QList<QSharedPointer<QueryTypeStrategy>> strategies;
};

DPSEARCH_END_NAMESPACE

#endif   // QUERYSTRATEGIES_H
