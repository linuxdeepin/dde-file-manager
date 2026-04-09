// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacydocutilsextractor.h"

#include "utils/docutils.h"

SERVICETEXTINDEX_BEGIN_NAMESPACE

IndexExtractionResult LegacyDocUtilsExtractor::extract(const QString &filePath, size_t maxBytes) const
{
    const auto contentOpt = DocUtils::extractFileContent(filePath, maxBytes);
    if (!contentOpt.has_value()) {
        return { false, QString(), QStringLiteral("DocUtils extraction failed") };
    }

    return { true, contentOpt.value().trimmed(), QString() };
}

SERVICETEXTINDEX_END_NAMESPACE
