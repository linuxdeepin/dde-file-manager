// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSEXTRACTOR_H
#define PROCESSEXTRACTOR_H

#include "indexextractor.h"

#include <QScopedPointer>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class ProcessExtractorPrivate;

class ProcessExtractor : public IndexExtractor
{
public:
    ProcessExtractor();
    ~ProcessExtractor() override;

    Q_DISABLE_COPY_MOVE(ProcessExtractor)

    IndexExtractionResult extract(const QString &filePath, size_t maxBytes = 0) const override;

private:
    const QScopedPointer<ProcessExtractorPrivate> d;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // PROCESSEXTRACTOR_H
