// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMEDOCUMENTBUILDER_H
#define FILENAMEDOCUMENTBUILDER_H

#include "indexdocumentbuilder.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FileNameDocumentBuilder : public IndexDocumentBuilder
{
public:
    Lucene::DocumentPtr build(const QString &filePath,
                              const QString &text,
                              const BuilderOptions &options = {}) const override;

    // is_hidden 由完整路径派生：目录改名/移入移出隐藏目录树时按新路径重算。
    // 其余字段（file_name/pinyin/file_ext/file_type 等）由 basename 派生，
    // 目录改名不改变 basename，copy 保留即可，避免逐文档重建的开销
    std::list<PathDerivedFieldSpec> pathDerivedFields() const override;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FILENAMEDOCUMENTBUILDER_H
