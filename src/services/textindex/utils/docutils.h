// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUTILS_H
#define DOCUTILS_H

#include "service_textindex_global.h"

#include <QString>

#include <optional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class DTextEncoding;

namespace DocUtils {

/**
 * @brief Detects file encoding
 * @param filePath Path to the file
 * @return Detected encoding name
 */
QByteArray detectFileEncoding(const QString &filePath);

/**
 * @brief Converts text from one encoding to another
 * @param input Input text as QByteArray
 * @param output Output text as QByteArray
 * @param toEncoding Target encoding
 * @param fromEncoding Source encoding
 * @return True if conversion was successful
 */
bool convertTextEncoding(const QByteArray &input, QByteArray &output,
                         const QString &toEncoding, const QString &fromEncoding);

/**
 * @brief Converts the input content to a UTF-8 encoded QString
 * @param content Input content
 * @param fromEncoding Source encoding
 * @return The converted UTF-8 encoded QString, returns an empty optional if conversion fails
 */
std::optional<QString> convertToUtf8(const QByteArray &content, const QString &fromEncoding);

/**
 * @brief Extracts text content from a file
 * @param filePath Path to the file
 * @return Extracted text content or empty optional if extraction failed
 */
std::optional<QString> extractFileContent(const QString &filePath);

/**
 * @brief Checks if a file is an HTML-style document
 * @param filePath Path to the file
 * @return True if the file is an HTML-style document
 */
bool isHtmlStyleDocument(const QString &filePath);

/**
 * @brief Extracts text from HTML-style documents using QTextDocument
 * @param filePath Path to the file
 * @return Extracted text content or empty optional if extraction failed
 */
std::optional<QString> extractHtmlContent(const QString &filePath);

}   // namespace DocUtils

SERVICETEXTINDEX_END_NAMESPACE

#endif   // DOCUTILS_H
