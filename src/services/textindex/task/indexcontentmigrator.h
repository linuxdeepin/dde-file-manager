// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXCONTENTMIGRATOR_H
#define INDEXCONTENTMIGRATOR_H

#include "service_textindex_global.h"

#include <lucene++/LuceneHeaders.h>

#include <QString>
#include <optional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexProfile;

/**
 * @brief 索引内容迁移器 — 在版本升级重建索引时从旧索引按需复用 content 字段
 *
 * 使用方式:
 *   1. migrator.prepare(indexDir, profile)  — rename 旧索引 + 打开 reader
 *   2. migrator.lookupContent(filePath)     — 对每个文件按需查询（per-file TermQuery）
 *   3. migrator.cleanup()                   — 删除 .old 目录 + 关闭 reader
 *
 * hasResidue() 用于检测 .old 残留（崩溃恢复时 UpdateIndexHandler 据此决定是否创建 migrator）。
 *
 * cleanup() 仅在任务成功完成时显式调用，中断/析构时不自动清理 —— .old 保留供
 * 下次启动恢复使用（reader 通过 Lucene++ 智能指针正常关闭）。
 */
class IndexContentMigrator
{
public:
    IndexContentMigrator() = default;

    // 禁止拷贝和移动
    IndexContentMigrator(const IndexContentMigrator &) = delete;
    IndexContentMigrator &operator=(const IndexContentMigrator &) = delete;
    IndexContentMigrator(IndexContentMigrator &&) = delete;
    IndexContentMigrator &operator=(IndexContentMigrator &&) = delete;

    /**
     * @brief 准备迁移：rename indexDir → indexDir.old，打开 IndexReader
     * @param indexDir 当前索引目录
     * @param profile  索引配置（提供字段名）
     * @return true 迁移就绪，false 降级（旧索引不存在或 rename 失败）
     */
    bool prepare(const QString &indexDir, const IndexProfile &profile);

    /**
     * @brief 从旧索引按路径查询内容
     * @param filePath 文件绝对路径
     * @return 三态结果：
     *   - 有值（非空 QString）：命中且 modifyTime 匹配，返回 content
     *   - 有值（空 QString）：命中且 modifyTime 匹配，但 content 字段确实为空
     *   - std::nullopt：未命中、校验失败或异常，应回退到 extract 流程
     *
     * 每次调用执行一次 TermQuery，O(1) 内存开销。
     * modifyTime 校验确保文件在索引重建间隔内未被修改。
     */
    std::optional<QString> lookupContent(const QString &filePath) const;

    /**
     * @brief 清理 .old 目录并关闭 reader
     */
    void cleanup();

    /**
     * @brief 迁移是否处于活跃状态
     */
    bool isActive() const;

    /**
     * @brief 检测指定索引目录是否存在 .old 残留
     */
    static bool hasResidue(const QString &indexDir);

private:
    QString m_oldIndexDir;
    Lucene::IndexReaderPtr m_reader;
    Lucene::SearcherPtr m_searcher;
    const wchar_t *m_pathField { nullptr };
    const wchar_t *m_contentField { nullptr };
    const wchar_t *m_modifyTimeField { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXCONTENTMIGRATOR_H