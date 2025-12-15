// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docutils.h"

#include <DTextEncoding>

#include <docparser.h>

#include <QSet>
#include <QFile>
#include <QTextDocument>
#include <QFileInfo>
#include <QDebug>
#include <QMimeDatabase>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace DocUtils {

QByteArray detectFileEncoding(const QString &filePath)
{
    return Dtk::Core::DTextEncoding::detectFileEncoding(filePath);
}

bool convertTextEncoding(const QByteArray &input, QByteArray &output,
                         const QString &toEncoding, const QString &fromEncoding)
{
    return Dtk::Core::DTextEncoding::convertTextEncodingEx(const_cast<QByteArray &>(input), output,
                                                           toEncoding.toUtf8(), fromEncoding.toUtf8());
}

QString getFileEncoding(const QString &filePath)
{
    // Check if file is a text file before detecting encoding
    QMimeDatabase mimeDb;
    QString mimeTypeName = mimeDb.mimeTypeForFile(filePath).name();

    // Detect encoding only for text files, otherwise default to UTF-8
    if (mimeTypeName.startsWith("text/")) {
        return QString::fromUtf8(detectFileEncoding(filePath));
    } else {
        return QString("utf-8");
    }
}

std::optional<QString> convertToUtf8(const QByteArray &content, const QString &fromEncoding)
{
    if (fromEncoding.toLower() == "utf-8") {
        return QString::fromUtf8(content);
    } else {
        QByteArray out;
        if (convertTextEncoding(content, out, "utf-8", fromEncoding)) {
            return QString::fromUtf8(out);
        } else {
            fmWarning() << "DocUtils: Failed to convert encoding from" << fromEncoding << "to UTF-8";
            return std::nullopt;
        }
    }
}

bool isHtmlStyleDocument(const QString &filePath)
{
    // HTML-style document extensions
    static const QSet<QString> kHtmlStyleExtensions = { "html", "htm", "xml", "uof" };

    QFileInfo fileInfo(filePath);
    return kHtmlStyleExtensions.contains(fileInfo.suffix().toLower());
}

std::optional<QString> extractHtmlContent(const QString &filePath, size_t maxBytes)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        fmWarning() << "DocUtils: Failed to open file for HTML extraction:" << filePath;
        return std::nullopt;
    }

    QByteArray htmlBytes;
    if (maxBytes > 0) {
        // Check file size first
        qint64 fileSize = file.size();
        if (fileSize <= static_cast<qint64>(maxBytes)) {
            // File is smaller than or equal to maxBytes, read all content
            htmlBytes = file.readAll();
        } else {
            // File is larger than maxBytes, truncate
            htmlBytes = file.read(static_cast<qint64>(maxBytes));
            fmDebug() << "DocUtils: HTML file truncated from" << fileSize << "to" << maxBytes << "bytes:" << filePath;
        }
    } else {
        // Read all content when maxBytes is 0 (no limit)
        htmlBytes = file.readAll();
    }
    file.close();

    // Get file encoding (text files: detected, non-text files: UTF-8)
    QString fromEncoding = getFileEncoding(filePath);

    auto convertedContent = convertToUtf8(htmlBytes, fromEncoding);
    if (!convertedContent) {
        return std::nullopt;
    }

    QTextDocument doc;
    doc.setHtml(convertedContent.value());
    QString plainText = doc.toPlainText();

    // Check if extraction was successful (non-empty result)
    if (plainText.trimmed().isEmpty())
        return std::nullopt;

    return plainText;
}

std::optional<QString> extractFileContent(const QString &filePath, size_t maxBytes)
{
    fmInfo() << "About to extract file: " << filePath;
    // First try HTML extraction for HTML-style documents
    if (isHtmlStyleDocument(filePath)) {
        auto htmlContent = extractHtmlContent(filePath, maxBytes);
        if (htmlContent) {
            return htmlContent;
        }
        // If HTML extraction failed, fall back to DocParser
    }

    // Use DocParser for all other files or as fallback
    try {
        // Get file encoding (text files: detected, non-text files: UTF-8)
        QString fromEncoding = getFileEncoding(filePath);

        // Convert file content using the new DocParser interface with maxBytes support
        std::string stdContents;
        if (maxBytes > 0) {
            // Use the new convertFile interface with maxBytes parameter
            stdContents = DocParser::convertFile(filePath.toStdString(), maxBytes);
        } else {
            // Use the original convertFile interface without truncation
            stdContents = DocParser::convertFile(filePath.toStdString());
        }

        QByteArray contentBytes(stdContents.c_str(), stdContents.length());
        return convertToUtf8(contentBytes, fromEncoding);
    } catch (const std::exception &e) {
        fmWarning() << "DocUtils: Exception while extracting file content from" << filePath << ":" << e.what();
        return std::nullopt;
    }

    return std::nullopt;
}

Lucene::DocumentPtr copyFieldsExcept(const Lucene::DocumentPtr &sourceDoc,
                                     std::initializer_list<Lucene::String> excludeFieldNames)
{
    using namespace Lucene;

    if (!sourceDoc) {
        return nullptr;
    }

    DocumentPtr newDoc = newLucene<Document>();
    Collection<FieldablePtr> fields = sourceDoc->getFields();

    for (Collection<FieldablePtr>::iterator fieldIt = fields.begin(); fieldIt != fields.end(); ++fieldIt) {
        FieldablePtr fieldable = *fieldIt;
        String fieldName = fieldable->name();

        // Check if field name is in the exclude list
        bool shouldExclude = false;
        for (const auto &excludeName : excludeFieldNames) {
            if (fieldName == excludeName) {
                shouldExclude = true;
                break;
            }
        }

        if (!shouldExclude) {
            // Copy field with original properties
            newDoc->add(newLucene<Field>(fieldName, fieldable->stringValue(),
                                         fieldable->isStored() ? Field::STORE_YES : Field::STORE_NO,
                                         fieldable->isIndexed() ? (fieldable->isTokenized() ? Field::INDEX_ANALYZED : Field::INDEX_NOT_ANALYZED) : Field::INDEX_NO));
        }
    }

    return newDoc;
}

}   // namespace DocUtils

SERVICETEXTINDEX_END_NAMESPACE
