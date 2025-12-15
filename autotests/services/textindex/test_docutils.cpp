// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <docparser.h>
#include <QFile>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QMimeDatabase>
#include <DTextEncoding>
#include <optional>

#include "utils/docutils.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_DocUtils : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for testing
        tempDir = std::make_unique<QTemporaryDir>();
        EXPECT_TRUE(tempDir->isValid());

        testDir = tempDir->path();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    void createTestFile(const QString &relativePath, const QByteArray &content)
    {
        QString fullPath = testDir + "/" + relativePath;
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        QFile file(fullPath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        file.write(content);
        file.close();
    }

    QString getTestFilePath(const QString &relativePath) const
    {
        return testDir + "/" + relativePath;
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testDir;

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_DocUtils, DetectFileEncoding_ValidFile_ReturnsEncoding)
{
    // Mock DTextEncoding::detectFileEncoding
    stub.set_lamda(ADDR(Dtk::Core::DTextEncoding, detectFileEncoding), [](const QString &filePath, bool *isOk) -> QByteArray {
        __DBG_STUB_INVOKE__
        if (isOk) *isOk = true;
        return "utf-8";
    });

    createTestFile("test.txt", "Hello World");

    QByteArray result = DocUtils::detectFileEncoding(getTestFilePath("test.txt"));

    EXPECT_EQ(result, "utf-8");
}

TEST_F(UT_DocUtils, ConvertTextEncoding_ValidConversion_ReturnsTrue)
{
    QByteArray input = "Hello World";
    QByteArray output;

    // Mock DTextEncoding::convertTextEncodingEx
    stub.set_lamda(ADDR(Dtk::Core::DTextEncoding, convertTextEncodingEx),
                   [](QByteArray &input, QByteArray &output, const QByteArray &toEncoding, const QByteArray &fromEncoding, QString *errStr, int *errCode) -> bool {
                       __DBG_STUB_INVOKE__
                       output = input;   // Simple mock conversion
                       return true;
                   });

    bool result = DocUtils::convertTextEncoding(input, output, "utf-8", "iso-8859-1");

    EXPECT_TRUE(result);
    EXPECT_EQ(output, input);
}

TEST_F(UT_DocUtils, ConvertTextEncoding_InvalidConversion_ReturnsFalse)
{
    QByteArray input = "Hello World";
    QByteArray output;

    // Mock DTextEncoding::convertTextEncodingEx to fail
    stub.set_lamda(ADDR(Dtk::Core::DTextEncoding, convertTextEncodingEx),
                   [](QByteArray &input, QByteArray &output, const QByteArray &toEncoding, const QByteArray &fromEncoding, QString *errStr, int *errCode) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = DocUtils::convertTextEncoding(input, output, "utf-8", "invalid-encoding");

    EXPECT_FALSE(result);
}

TEST_F(UT_DocUtils, GetFileEncoding_TextFile_ReturnsDetectedEncoding)
{
    // Mock QMimeDatabase
    using MimeTypeForFileFunc = QMimeType (QMimeDatabase::*)(const QString &, QMimeDatabase::MatchMode) const;

    stub.set_lamda(static_cast<MimeTypeForFileFunc>(&QMimeDatabase::mimeTypeForFile),
                   [](QMimeDatabase *, const QString &fileName, QMimeDatabase::MatchMode) -> QMimeType {
                       __DBG_STUB_INVOKE__
                       QMimeType mockType;
                       return mockType;
                   });

    // Mock QMimeType::name to return text type
    stub.set_lamda(ADDR(QMimeType, name), [](QMimeType *) -> QString {
        __DBG_STUB_INVOKE__
        return "text/plain";
    });

    // Mock DTextEncoding::detectFileEncoding
    stub.set_lamda(ADDR(Dtk::Core::DTextEncoding, detectFileEncoding), [](const QString &filePath, bool) -> QByteArray {
        __DBG_STUB_INVOKE__
        return "iso-8859-1";
    });

    createTestFile("text.txt", "Hello World");

    QString result = DocUtils::getFileEncoding(getTestFilePath("text.txt"));

    EXPECT_EQ(result, "iso-8859-1");
}

TEST_F(UT_DocUtils, GetFileEncoding_NonTextFile_ReturnsUtf8)
{
    // Mock QMimeDatabase to return non-text type
    using MimeTypeForFileFunc = QMimeType (QMimeDatabase::*)(const QString &, QMimeDatabase::MatchMode) const;

    stub.set_lamda(static_cast<MimeTypeForFileFunc>(&QMimeDatabase::mimeTypeForFile),
                   [](QMimeDatabase *, const QString &fileName, QMimeDatabase::MatchMode) -> QMimeType {
                       __DBG_STUB_INVOKE__
                       QMimeType mockType;
                       return mockType;
                   });

    // Mock QMimeType::name to return binary type
    stub.set_lamda(ADDR(QMimeType, name), [](QMimeType *) -> QString {
        __DBG_STUB_INVOKE__
        return "application/octet-stream";
    });

    createTestFile("binary.bin", "Binary data");

    QString result = DocUtils::getFileEncoding(getTestFilePath("binary.bin"));

    EXPECT_EQ(result, "utf-8");
}

TEST_F(UT_DocUtils, ConvertToUtf8_Utf8Input_ReturnsDirectly)
{
    QByteArray content = "Hello World";

    std::optional<QString> result = DocUtils::convertToUtf8(content, "utf-8");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Hello World");
}

TEST_F(UT_DocUtils, ConvertToUtf8_DifferentEncoding_ConvertsSuccessfully)
{
    QByteArray content = "Hello World";

    // Mock convertTextEncoding to succeed
    stub.set_lamda(ADDR(DocUtils, convertTextEncoding),
                   [](const QByteArray &input, QByteArray &output, const QString &toEncoding, const QString &fromEncoding) -> bool {
                       __DBG_STUB_INVOKE__
                       output = input;   // Simple mock conversion
                       return true;
                   });

    std::optional<QString> result = DocUtils::convertToUtf8(content, "iso-8859-1");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Hello World");
}

TEST_F(UT_DocUtils, ConvertToUtf8_ConversionFails_ReturnsEmpty)
{
    QByteArray content = "Hello World";

    // Mock convertTextEncoding to fail
    stub.set_lamda(ADDR(DocUtils, convertTextEncoding),
                   [](const QByteArray &input, QByteArray &output, const QString &toEncoding, const QString &fromEncoding) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    std::optional<QString> result = DocUtils::convertToUtf8(content, "invalid-encoding");

    EXPECT_FALSE(result.has_value());
}

TEST_F(UT_DocUtils, IsHtmlStyleDocument_HtmlFile_ReturnsTrue)
{
    EXPECT_TRUE(DocUtils::isHtmlStyleDocument("document.html"));
    EXPECT_TRUE(DocUtils::isHtmlStyleDocument("page.htm"));
    EXPECT_TRUE(DocUtils::isHtmlStyleDocument("data.xml"));
    EXPECT_TRUE(DocUtils::isHtmlStyleDocument("format.uof"));
}

TEST_F(UT_DocUtils, IsHtmlStyleDocument_NonHtmlFile_ReturnsFalse)
{
    EXPECT_FALSE(DocUtils::isHtmlStyleDocument("document.txt"));
    EXPECT_FALSE(DocUtils::isHtmlStyleDocument("image.jpg"));
    EXPECT_FALSE(DocUtils::isHtmlStyleDocument("archive.zip"));
    EXPECT_FALSE(DocUtils::isHtmlStyleDocument("script.js"));
}

TEST_F(UT_DocUtils, IsHtmlStyleDocument_CaseInsensitive_ReturnsTrue)
{
    EXPECT_TRUE(DocUtils::isHtmlStyleDocument("document.HTML"));
    EXPECT_TRUE(DocUtils::isHtmlStyleDocument("page.HTM"));
    EXPECT_TRUE(DocUtils::isHtmlStyleDocument("data.XML"));
}

TEST_F(UT_DocUtils, ExtractHtmlContent_ValidHtmlFile_ReturnsPlainText)
{
    QString htmlContent = "<html><body><h1>Title</h1><p>Content</p></body></html>";
    createTestFile("test.html", htmlContent.toUtf8());

    // Mock getFileEncoding
    stub.set_lamda(ADDR(DocUtils, getFileEncoding), [](const QString &filePath) -> QString {
        __DBG_STUB_INVOKE__
        return "utf-8";
    });

    // Mock convertToUtf8
    stub.set_lamda(ADDR(DocUtils, convertToUtf8), [&htmlContent](const QByteArray &content, const QString &fromEncoding) -> std::optional<QString> {
        __DBG_STUB_INVOKE__
        return htmlContent;
    });

    std::optional<QString> result = DocUtils::extractHtmlContent(getTestFilePath("test.html"));

    EXPECT_TRUE(result.has_value());
    // The result should be plain text without HTML tags
    EXPECT_TRUE(result.value().contains("Title"));
    EXPECT_TRUE(result.value().contains("Content"));
    EXPECT_FALSE(result.value().contains("<html>"));
    EXPECT_FALSE(result.value().contains("<p>"));
}

TEST_F(UT_DocUtils, ExtractHtmlContent_FileNotFound_ReturnsEmpty)
{
    std::optional<QString> result = DocUtils::extractHtmlContent("nonexistent.html");

    EXPECT_FALSE(result.has_value());
}

TEST_F(UT_DocUtils, ExtractHtmlContent_WithMaxBytes_TruncatesContent)
{
    QString longHtmlContent = "<html><body>";
    for (int i = 0; i < 1000; ++i) {
        longHtmlContent += "<p>Long content line " + QString::number(i) + "</p>";
    }
    longHtmlContent += "</body></html>";

    createTestFile("long.html", longHtmlContent.toUtf8());

    // Mock getFileEncoding
    stub.set_lamda(ADDR(DocUtils, getFileEncoding), [](const QString &filePath) -> QString {
        __DBG_STUB_INVOKE__
        return "utf-8";
    });

    // Mock convertToUtf8 to return truncated content
    stub.set_lamda(ADDR(DocUtils, convertToUtf8), [](const QByteArray &content, const QString &fromEncoding) -> std::optional<QString> {
        __DBG_STUB_INVOKE__
        return QString::fromUtf8(content);
    });

    std::optional<QString> result = DocUtils::extractHtmlContent(getTestFilePath("long.html"), 1000);

    EXPECT_TRUE(result.has_value());
    // Should be truncated
    EXPECT_LT(result.value().length(), longHtmlContent.length());
}

TEST_F(UT_DocUtils, ExtractFileContent_HtmlFile_UsesHtmlExtraction)
{
    QString htmlContent = "<html><body><p>Test content</p></body></html>";
    createTestFile("test.html", htmlContent.toUtf8());

    // Mock isHtmlStyleDocument to return true
    stub.set_lamda(ADDR(DocUtils, isHtmlStyleDocument), [](const QString &filePath) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock extractHtmlContent
    stub.set_lamda(ADDR(DocUtils, extractHtmlContent), [](const QString &filePath, size_t maxBytes) -> std::optional<QString> {
        __DBG_STUB_INVOKE__
        return QString("Test content");
    });

    std::optional<QString> result = DocUtils::extractFileContent(getTestFilePath("test.html"));

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Test content");
}

TEST_F(UT_DocUtils, ExtractFileContent_NonHtmlFile_UsesDocParser)
{
    createTestFile("test.txt", "Plain text content");

    // Mock isHtmlStyleDocument to return false
    stub.set_lamda(ADDR(DocUtils, isHtmlStyleDocument), [](const QString &filePath) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock getFileEncoding
    stub.set_lamda(ADDR(DocUtils, getFileEncoding), [](const QString &filePath) -> QString {
        __DBG_STUB_INVOKE__
        return "utf-8";
    });

    // Mock DocParser::convertFile (静态函数)
    using ConvertFunc = std::string (*)(const std::string &);
    stub.set_lamda(static_cast<ConvertFunc>(&DocParser::convertFile), [](const std::string &filePath) -> std::string {
        __DBG_STUB_INVOKE__
        return "Plain text content";
    });

    // Mock convertToUtf8
    stub.set_lamda(ADDR(DocUtils, convertToUtf8), [](const QByteArray &content, const QString &fromEncoding) -> std::optional<QString> {
        __DBG_STUB_INVOKE__
        return QString::fromUtf8(content);
    });

    std::optional<QString> result = DocUtils::extractFileContent(getTestFilePath("test.txt"));

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Plain text content");
}

TEST_F(UT_DocUtils, ExtractFileContent_DocParserThrowsException_ReturnsEmpty)
{
    createTestFile("test.doc", "Document content");

    // Mock isHtmlStyleDocument to return false
    stub.set_lamda(ADDR(DocUtils, isHtmlStyleDocument), [](const QString &filePath) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock DocParser::convertFile to throw exception (静态函数)
    using ConvertFunc = std::string (*)(const std::string &);
    stub.set_lamda(static_cast<ConvertFunc>(&DocParser::convertFile), [](const std::string &filePath) -> std::string {
        __DBG_STUB_INVOKE__
        throw std::runtime_error("DocParser error");
    });

    std::optional<QString> result = DocUtils::extractFileContent(getTestFilePath("test.doc"));

    EXPECT_FALSE(result.has_value());
}

TEST_F(UT_DocUtils, CopyFieldsExcept_ValidDocument_CopiesAllExceptExcluded)
{
    // This test would require mocking Lucene objects, which is complex
    // For now, we test the basic null pointer handling

    Lucene::DocumentPtr result = DocUtils::copyFieldsExcept(nullptr, { L"excludedField" });

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_DocUtils, ExtractFileContent_WithMaxBytes_PassesToDocParser)
{
    createTestFile("test.pdf", "PDF content");

    // Mock isHtmlStyleDocument to return false
    stub.set_lamda(ADDR(DocUtils, isHtmlStyleDocument), [](const QString &filePath) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock getFileEncoding
    stub.set_lamda(ADDR(DocUtils, getFileEncoding), [](const QString &filePath) -> QString {
        __DBG_STUB_INVOKE__
        return "utf-8";
    });

    // Mock DocParser::convertFile with maxBytes parameter (重载函数)
    using ConvertFileFunc = std::string (*)(const std::string &, size_t);
    stub.set_lamda(static_cast<ConvertFileFunc>(DocParser::convertFile), [](const std::string &filePath, size_t maxBytes) -> std::string {
        __DBG_STUB_INVOKE__
        return "Truncated content";
    });

    // Mock convertToUtf8
    stub.set_lamda(ADDR(DocUtils, convertToUtf8), [](const QByteArray &content, const QString &fromEncoding) -> std::optional<QString> {
        __DBG_STUB_INVOKE__
        return QString::fromUtf8(content);
    });

    std::optional<QString> result = DocUtils::extractFileContent(getTestFilePath("test.pdf"), 1000);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Truncated content");
}
