// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEGACYDOCUTILSEXTRACTOR_H
#define LEGACYDOCUTILSEXTRACTOR_H

#include "indexextractor.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

class LegacyDocUtilsExtractor : public IndexExtractor
{
public:
    IndexExtractionResult extract(const QString &filePath, size_t maxBytes = 0) const override;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // LEGACYDOCUTILSEXTRACTOR_H
