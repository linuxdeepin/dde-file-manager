// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textextractor.h"

#include <DTextEncoding>

#include <docparser.h>

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QSet>
#include <QTextDocument>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

namespace {

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
    QMimeDatabase mimeDb;
    QString mimeTypeName = mimeDb.mimeTypeForFile(filePath).name();

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
            fmWarning() << "TextExtractor: Failed to convert encoding from" << fromEncoding << "to UTF-8";
            return std::nullopt;
        }
    }
}

bool isHtmlStyleDocument(const QString &filePath)
{
    static const QSet<QString> kHtmlStyleExtensions = { "html", "htm", "xml", "uof" };

    QFileInfo fileInfo(filePath);
    return kHtmlStyleExtensions.contains(fileInfo.suffix().toLower());
}

std::optional<QString> extractHtmlContent(const QString &filePath, size_t maxBytes)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        fmWarning() << "TextExtractor: Failed to open file for HTML extraction:" << filePath;
        return std::nullopt;
    }

    QByteArray htmlBytes;
    if (maxBytes > 0) {
        qint64 fileSize = file.size();
        if (fileSize <= static_cast<qint64>(maxBytes)) {
            htmlBytes = file.readAll();
        } else {
            htmlBytes = file.read(static_cast<qint64>(maxBytes));
            fmDebug() << "TextExtractor: HTML file truncated from" << fileSize << "to" << maxBytes << "bytes:" << filePath;
        }
    } else {
        htmlBytes = file.readAll();
    }
    file.close();

    QString fromEncoding = getFileEncoding(filePath);

    auto convertedContent = convertToUtf8(htmlBytes, fromEncoding);
    if (!convertedContent) {
        return std::nullopt;
    }

    QTextDocument doc;
    doc.setHtml(convertedContent.value());
    QString plainText = doc.toPlainText();

    if (plainText.trimmed().isEmpty())
        return std::nullopt;

    return plainText;
}

}   // namespace

std::optional<QByteArray> TextExtractor::extract(const QString &filePath, size_t maxBytes)
{
    fmDebug() << "TextExtractor: Extracting file:" << filePath;

    // First try HTML extraction for HTML-style documents
    if (isHtmlStyleDocument(filePath)) {
        auto htmlContent = extractHtmlContent(filePath, maxBytes);
        if (htmlContent) {
            return htmlContent->toUtf8();
        }
    }

    // Use DocParser for all other filefs
    try {
        QString fromEncoding = getFileEncoding(filePath);

        std::string stdContents;
        if (maxBytes > 0) {
            stdContents = DocParser::convertFile(filePath.toStdString(), maxBytes);
        } else {
            stdContents = DocParser::convertFile(filePath.toStdString());
        }

        QByteArray contentBytes(stdContents.c_str(), stdContents.length());
        auto result = convertToUtf8(contentBytes, fromEncoding);

        if (result) {
            return result->toUtf8();
        }

        return std::nullopt;
    } catch (const std::exception &e) {
        fmWarning() << "TextExtractor: Exception while extracting file content from" << filePath << ":" << e.what();
        return std::nullopt;
    }
}

bool TextExtractor::isTextDocument(const QString &filePath)
{
    QMimeDatabase mimeDb;
    QString mimeTypeName = mimeDb.mimeTypeForFile(filePath).name();

    // Check for text/* mimetypes
    if (mimeTypeName.startsWith("text/")) {
        return true;
    }

    // Check for common document types
    static const QSet<QString> kDocumentMimeTypes = {
        "application/pdf",
        "application/vnd.oasis.opendocument.text",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "application/msword",
        "application/rtf",
        "application/epub+zip"
    };

    if (kDocumentMimeTypes.contains(mimeTypeName)) {
        return true;
    }

    // Check file extension for HTML-style documents
    if (isHtmlStyleDocument(filePath)) {
        return true;
    }

    return false;
}

EXTRACTOR_PLUGIN_END_NAMESPACE
