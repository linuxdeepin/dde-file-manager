// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTRACTORTYPES_H
#define EXTRACTORTYPES_H

#include "extractor_global.h"

#include <QByteArray>
#include <QString>
#include <QVector>

EXTRACTOR_BEGIN_NAMESPACE

/**
 * @brief Status codes for extractor IPC communication
 */
enum class ExtractorStatus : quint8 {
    Started = 'S',     // Start processing a file
    Finished = 'F',    // Successfully completed
    Failed = 'f',      // Processing failed
    Data = 'D',        // Data ready (path + data)
    BatchDone = 'B'    // Batch completed
};

/**
 * @brief Convert ExtractorStatus to string for logging
 */
inline const char* statusToString(ExtractorStatus status)
{
    switch (status) {
    case ExtractorStatus::Started:
        return "Started";
    case ExtractorStatus::Finished:
        return "Finished";
    case ExtractorStatus::Failed:
        return "Failed";
    case ExtractorStatus::Data:
        return "Data";
    case ExtractorStatus::BatchDone:
        return "BatchDone";
    default:
        return "Unknown";
    }
}

/**
 * @brief Result of a single file extraction
 */
struct ExtractionResult {
    QString filePath;
    bool success = false;
    QByteArray data;
    QString error;
};

/**
 * @brief Batch extraction results
 */
struct BatchResult {
    QVector<ExtractionResult> results;
    int successCount = 0;
    int failureCount = 0;
};

EXTRACTOR_END_NAMESPACE

#endif   // EXTRACTORTYPES_H
