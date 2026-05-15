// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCRDEDUPLICATION_H
#define OCRDEDUPLICATION_H

#include "service_textindex_global.h"

#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace OcrDeduplication {

/**
 * @brief Attempt to find existing OCR text by file checksum
 *
 * Opens the OCR index read-only, searches for a document with matching
 * checksum, and returns the ocr_contents text if found.
 *
 * This is a best-effort optimization: on any failure (index unavailable,
 * corrupt, etc.) it returns an empty string so normal OCR extraction proceeds.
 *
 * @param checksum  MD5 hex digest of the file
 * @param ocrIndexDir  Path to the OCR Lucene index directory
 * @return The OCR text if a matching document was found, empty string otherwise
 */
QString lookupByTextChecksum(const QString &checksum, const QString &ocrIndexDir);

}   // namespace OcrDeduplication

SERVICETEXTINDEX_END_NAMESPACE

#endif   // OCRDEDUPLICATION_H
