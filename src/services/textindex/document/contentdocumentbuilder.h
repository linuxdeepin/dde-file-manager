// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTENTDOCUMENTBUILDER_H
#define CONTENTDOCUMENTBUILDER_H

#include "indexdocumentbuilder.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

class ContentDocumentBuilder : public IndexDocumentBuilder
{
public:
    Lucene::DocumentPtr build(const QString &filePath, const QString &text) const override;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // CONTENTDOCUMENTBUILDER_H
