// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYWORDEXTRACTOR_H
#define KEYWORDEXTRACTOR_H

#include "dfmplugin_workspace_global.h"

#include <QStringList>
#include <QUrl>
#include <QSharedPointer>
#include <QRegularExpression>

namespace dfmplugin_workspace {

/**
 * @brief 关键字提取策略的抽象基类
 * 
 * 使用策略模式来处理不同类型的关键字提取，
 * 符合开闭原则，便于扩展新的提取类型。
 */
class KeywordExtractionStrategy
{
public:
    virtual ~KeywordExtractionStrategy() = default;

    /**
     * @brief 从搜索关键字中提取用于高亮的关键字列表
     * @param keyword 原始搜索关键字
     * @return 提取出的关键字列表，用于高亮显示
     */
    virtual QStringList extractKeywords(const QString &keyword) const = 0;

    /**
     * @brief 判断是否能处理指定的关键字
     * @param keyword 搜索关键字
     * @return 如果能处理返回true，否则返回false
     */
    virtual bool canHandle(const QString &keyword) const = 0;

    /**
     * @brief 获取策略的优先级，数值越小优先级越高
     * @return 优先级数值
     */
    virtual int priority() const = 0;
};

/**
 * @brief 简单关键字提取策略
 * 
 * 处理不包含空白字符和通配符的简单关键词。
 * 这是默认的回退策略，优先级最低。
 */
class SimpleKeywordStrategy : public KeywordExtractionStrategy
{
public:
    QStringList extractKeywords(const QString &keyword) const override;
    bool canHandle(const QString &keyword) const override;
    int priority() const override { return 100; } // 最低优先级
};

/**
 * @brief 布尔关键字提取策略
 * 
 * 处理包含空白字符的多关键词搜索，按空白字符分割。
 */
class BooleanKeywordStrategy : public KeywordExtractionStrategy
{
public:
    QStringList extractKeywords(const QString &keyword) const override;
    bool canHandle(const QString &keyword) const override;
    int priority() const override { return 10; } // 高优先级

private:
    const QRegularExpression whitespacePattern { "\\s" };
    const QRegularExpression whitespaceDelimiter { "\\s+" };
};

/**
 * @brief 通配符关键字提取策略
 * 
 * 处理包含通配符（* 和 ?）的搜索查询，提取通配符之间的有效关键字。
 * 例如："file*.deb" -> ["file", "deb"]
 */
class WildcardKeywordStrategy : public KeywordExtractionStrategy
{
public:
    QStringList extractKeywords(const QString &keyword) const override;
    bool canHandle(const QString &keyword) const override;
    int priority() const override { return 20; } // 中等优先级

private:
    /**
     * @brief 从通配符模式中提取有效的关键字部分
     * @param pattern 包含通配符的模式
     * @return 提取出的关键字列表
     */
    QStringList extractFromWildcardPattern(const QString &pattern) const;
};

/**
 * @brief 关键字提取器工厂类
 * 
 * 负责根据关键词选择合适的策略来提取关键字。
 * 策略按优先级顺序进行检查。
 */
class KeywordExtractor
{
public:
    KeywordExtractor();
    ~KeywordExtractor() = default;

    /**
     * @brief 从URL查询参数中提取关键字
     * @param url 包含keyword查询参数的URL
     * @return 提取出的关键字列表
     */
    QStringList extractFromUrl(const QUrl &url) const;

    /**
     * @brief 从关键字字符串中提取关键字
     * @param keyword 关键字字符串
     * @return 提取出的关键字列表
     */
    QStringList extractFromKeyword(const QString &keyword) const;

    /**
     * @brief 注册新的关键字提取策略
     * @param strategy 策略指针
     */
    void registerStrategy(QSharedPointer<KeywordExtractionStrategy> strategy);

    /**
     * @brief 获取所有注册的策略（主要用于测试）
     * @return 策略列表
     */
    const QList<QSharedPointer<KeywordExtractionStrategy>>& getStrategies() const;

private:
    /**
     * @brief 按优先级排序策略列表
     */
    void sortStrategiesByPriority();

    QList<QSharedPointer<KeywordExtractionStrategy>> strategies;
};

/**
 * @brief 关键字提取器单例
 * 
 * 提供全局访问点，避免重复创建提取器实例。
 */
class KeywordExtractorManager
{
public:
    static KeywordExtractorManager& instance();
    
    /**
     * @brief 获取关键字提取器
     * @return 关键字提取器引用
     */
    KeywordExtractor& extractor();

private:
    KeywordExtractorManager() = default;
    ~KeywordExtractorManager() = default;
    KeywordExtractorManager(const KeywordExtractorManager&) = delete;
    KeywordExtractorManager& operator=(const KeywordExtractorManager&) = delete;

    KeywordExtractor keywordExtractor;
};

} // namespace dfmplugin_workspace

#endif // KEYWORDEXTRACTOR_H 