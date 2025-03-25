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

std::optional<QString> convertToUtf8(const QByteArray &content, const QString &fromEncoding)
{
    if (fromEncoding.toLower() == "utf-8") {
        return QString::fromUtf8(content);
    } else {
        QByteArray out;
        if (convertTextEncoding(content, out, "utf-8", fromEncoding)) {
            return QString::fromUtf8(out);
        } else {
            fmWarning() << "Failed to convert encoding from" << fromEncoding << "to UTF-8";
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

std::optional<QString> extractHtmlContent(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        fmWarning() << "Failed to open file for HTML extraction:" << filePath;
        return std::nullopt;
    }

    const QByteArray &htmlBytes = file.readAll();
    file.close();

    QString fromEncoding = detectFileEncoding(filePath);

    auto convertedContent = convertToUtf8(htmlBytes, fromEncoding);
    if (!convertedContent) {
        return std::nullopt;
    }

    QTextDocument doc;
    doc.setHtml(convertedContent.value());
    QString plainText = doc.toPlainText();

    // Check if extraction was successful (non-empty result)
    if (plainText.trimmed().isEmpty()) {
        fmWarning() << "HTML extraction resulted in empty content for:" << filePath;
        return std::nullopt;
    }

    return plainText;
}

std::optional<QString> extractFileContent(const QString &filePath)
{
    // First try HTML extraction for HTML-style documents
    if (isHtmlStyleDocument(filePath)) {
        auto htmlContent = extractHtmlContent(filePath);
        if (htmlContent) {
            return htmlContent;
        }
        // If HTML extraction failed, fall back to DocParser
    }

    // Use DocParser for all other files or as fallback
    try {
        // Detect encoding
        QString fromEncoding = detectFileEncoding(filePath);

        // Convert file content
        const std::string &stdContents = DocParser::convertFile(filePath.toStdString());

        QByteArray contentBytes(stdContents.c_str(), stdContents.length());
        return convertToUtf8(contentBytes, fromEncoding);
    } catch (const std::exception &e) {
        fmWarning() << "Exception while extracting file content:" << filePath << e.what();
        return std::nullopt;
    }

    return std::nullopt;
}

}   // namespace DocUtils

SERVICETEXTINDEX_END_NAMESPACE
