// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEXTRACTOR_H
#define TEXTEXTRACTOR_H

#include "extractor_plugin_global.h"

#include <QString>
#include <QByteArray>
#include <optional>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

/**
 * @brief TextExtractor handles text content extraction from various file formats.
 *
 * This class uses DocParser and DTextEncoding for content extraction,
 * similar to the textindex service's DocUtils::extractFileContent.
 */
class TextExtractor
{
public:
    /**
     * @brief Fallback maximum bytes to extract when dconfig is unavailable.
     */
    static constexpr size_t kDefaultMaxBytes = 10 * 1024 * 1024;   // 10 MB

    /**
     * @brief Extract text content from a file.
     *
     * @param filePath Path to the file
     * @return Extracted text as UTF-8 QByteArray, or nullopt on failure
     */
    static std::optional<QByteArray> extract(const QString &filePath);

    /**
     * @brief Extract text content from a file.
     *
     * @param filePath Path to the file
     * @param maxBytes Maximum bytes to extract (0 = no limit)
     * @return Extracted text as UTF-8 QByteArray, or nullopt on failure
     */
    static std::optional<QByteArray> extract(const QString &filePath, size_t maxBytes);

    /**
     * @brief Check if a file is likely a text-based document.
     *
     * @param filePath Path to the file
     * @return true if the file is a text document
     */
    static bool isTextDocument(const QString &filePath);
};

EXTRACTOR_PLUGIN_END_NAMESPACE

#endif   // TEXTEXTRACTOR_H
