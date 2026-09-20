// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXDOCUMENTBUILDER_H
#define INDEXDOCUMENTBUILDER_H

#include "document/builderoptions.h"
#include "service_textindex_global.h"

#include <QString>

#include <functional>
#include <list>

#include <lucene++/LuceneHeaders.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

/**
 * @brief 路径派生字段的重算规格
 *
 * 文档中由文件路径派生的字段（如 file_name/file_ext/is_hidden），在文件
 * 移动/重命名后需按新路径重算。rebuild 必须是纯元数据操作（无内容提取），
 * 返回空 FieldPtr 表示该字段在新路径下不存在（如空扩展名无 file_ext 字段）。
 */
struct PathDerivedFieldSpec
{
    const wchar_t *fieldName;
    std::function<Lucene::FieldPtr(const QString &newPath)> rebuild;
};

class IndexDocumentBuilder
{
public:
    virtual ~IndexDocumentBuilder() = default;

    virtual Lucene::DocumentPtr build(const QString &filePath,
                                      const QString &text,
                                      const BuilderOptions &options = {}) const = 0;

    /**
     * @brief 声明文档中由路径派生的部分字段及其重算方式
     *
     * 供 move 处理在保留文档其余字段（内容/校验和/时间戳等与路径无关的字段）
     * 的同时，按新路径重算这些派生字段。返回空列表表示无部分派生字段
     * （字段要么全部派生 → 走整体重建策略，要么全部与路径无关）。
     */
    virtual std::list<PathDerivedFieldSpec> pathDerivedFields() const { return {}; }
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXDOCUMENTBUILDER_H
