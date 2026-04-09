// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXDOCUMENTBUILDER_H
#define INDEXDOCUMENTBUILDER_H

#include "service_textindex_global.h"

#include <QString>

#include <lucene++/LuceneHeaders.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexDocumentBuilder
{
public:
    virtual ~IndexDocumentBuilder() = default;

    virtual Lucene::DocumentPtr build(const QString &filePath, const QString &text) const = 0;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXDOCUMENTBUILDER_H
