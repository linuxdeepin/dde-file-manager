// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCRDOCUMENTBUILDER_H
#define OCRDOCUMENTBUILDER_H

#include "indexdocumentbuilder.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

class OcrDocumentBuilder : public IndexDocumentBuilder
{
public:
    Lucene::DocumentPtr build(const QString &filePath,
                              const QString &text,
                              const BuilderOptions &options = {}) const override;

    // kFilename/kIsHidden/kFileExt 由路径派生，move 后按新路径重算
    std::list<PathDerivedFieldSpec> pathDerivedFields() const override;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // OCRDOCUMENTBUILDER_H
