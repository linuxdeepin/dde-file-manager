// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textextractor.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DTextEncoding>

#include <docparser.h>

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QTextDocument>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

namespace {

using DFMBASE_NAMESPACE::DConfigManager;

constexpr char kTextIndexSchema[] = "org.deepin.dde.file-manager.textindex";
constexpr char kSupportedFileExtensions[] = "supportedFileExtensions";
constexpr char kMaxIndexFileTruncationSizeMB[] = "maxIndexFileTruncationSizeMB";
constexpr int kDefaultMaxIndexFileTruncationSizeMB = 10;
constexpr int kMaxAllowedIndexFileTruncationSizeMB = 1024;

QStringList defaultSupportedExtensions()
{
    return {
        "rtf", "odt", "ods", "odp", "odg", "docx",
        "xlsx", "pptx", "ppsx", "md", "xls", "xlsb",
        "doc", "dot", "wps", "ppt", "pps", "txt",
        "pdf", "dps", "sh", "html", "htm", "xml",
        "xhtml", "dhtml", "shtm", "shtml", "json",
        "css", "yaml", "ini", "bat", "js", "sql",
        "uof", "ofd"
    };
}

void ensureTextIndexConfigLoaded()
{
    static const bool kConfigLoaded = [] {
        QString err;
        if (!DConfigManager::instance()->addConfig(QString::fromLatin1(kTextIndexSchema), &err))
            fmWarning() << "TextExtractor: Failed to load DConfig schema:" << err;
        return true;
    }();

    Q_UNUSED(kConfigLoaded)
}

QStringList supportedFileExtensions()
{
    ensureTextIndexConfigLoaded();

    return DConfigManager::instance()->value(QString::fromLatin1(kTextIndexSchema),
                                             QString::fromLatin1(kSupportedFileExtensions),
                                             QVariant::fromValue(defaultSupportedExtensions()))
            .toStringList();
}

size_t configuredMaxExtractBytes()
{
    ensureTextIndexConfigLoaded();

    const int truncationSizeMB = DConfigManager::instance()->value(QString::fromLatin1(kTextIndexSchema),
                                                                   QString::fromLatin1(kMaxIndexFileTruncationSizeMB),
                                                                   kDefaultMaxIndexFileTruncationSizeMB)
                                         .toInt();

    // Fix: keep extractor aligned with textindex defaults and guard invalid dconfig values.
    if (truncationSizeMB <= 0 || truncationSizeMB > kMaxAllowedIndexFileTruncationSizeMB) {
        fmWarning() << "TextExtractor: Invalid maxIndexFileTruncationSizeMB value:" << truncationSizeMB
                    << ", using default:" << kDefaultMaxIndexFileTruncationSizeMB;
        return TextExtractor::kDefaultMaxBytes;
    }

    return static_cast<size_t>(truncationSizeMB) * 1024 * 1024;
}

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

std::optional<QByteArray> TextExtractor::extract(const QString &filePath)
{
    return extract(filePath, configuredMaxExtractBytes());
}

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
    QFileInfo fileInfo(filePath);

    // Check for text/* mimetypes
    if (mimeTypeName.startsWith("text/"))
        return true;

    return supportedFileExtensions().contains(fileInfo.suffix().toLower());
}

EXTRACTOR_PLUGIN_END_NAMESPACE
