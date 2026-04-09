// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXEXTRACTOR_H
#define INDEXEXTRACTOR_H

#include "service_textindex_global.h"

#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

struct IndexExtractionResult
{
    bool success { false };
    QString text;
    QString error;
};

class IndexExtractor
{
public:
    virtual ~IndexExtractor() = default;

    virtual IndexExtractionResult extract(const QString &filePath, size_t maxBytes = 0) const = 0;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXEXTRACTOR_H
