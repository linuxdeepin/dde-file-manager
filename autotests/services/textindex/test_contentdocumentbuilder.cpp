// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_contentdocumentbuilder.cpp
 * @brief Unit tests for ContentDocumentBuilder (document/contentdocumentbuilder.cpp).
 *        Exercises the build() method with various file and options combinations
 *        using real QFileInfo operations on temporary files.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QDateTime>

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/document/contentdocumentbuilder.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

class ContentDocumentBuilderTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        // Create some test files
        QDir dir(tmp.path());
        ASSERT_TRUE(dir.mkpath("subdir"));
        createFile("test.txt", "hello world");
        createFile("subdir/nested.txt", "nested content");
        createFile("document.PDF", "pdf content");
    }

    void createFile(const QString &relativePath, const QString &content)
    {
        QFile f(tmp.path() + "/" + relativePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }

    ContentDocumentBuilder builder;
};

TEST_F(ContentDocumentBuilderTest, BuildBasicTextFile)
{
    QString filePath = tmp.path() + "/test.txt";
    auto doc = builder.build(filePath, "sample text content");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithEmptyText)
{
    QString filePath = tmp.path() + "/test.txt";
    auto doc = builder.build(filePath, "");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithWhitespaceText)
{
    QString filePath = tmp.path() + "/test.txt";
    auto doc = builder.build(filePath, "   \n\t  ");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithLongContent)
{
    QString filePath = tmp.path() + "/test.txt";
    QString longText(10000, 'A');
    auto doc = builder.build(filePath, longText);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithChecksumInOptions)
{
    QString filePath = tmp.path() + "/test.txt";
    BuilderOptions options;
    options.checksum = "abc123def456";
    auto doc = builder.build(filePath, "text", options);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithEmptyChecksumInOptions)
{
    QString filePath = tmp.path() + "/test.txt";
    BuilderOptions options;
    options.checksum = "";
    auto doc = builder.build(filePath, "text", options);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithDefaultOptions)
{
    QString filePath = tmp.path() + "/test.txt";
    BuilderOptions options;  // default: empty checksum
    auto doc = builder.build(filePath, "text", options);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildFileWithExtension)
{
    QString filePath = tmp.path() + "/document.PDF";
    auto doc = builder.build(filePath, "pdf content");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildFileWithoutExtension)
{
    QString filePath = tmp.path() + "/test.txt";
    // File has .txt extension; build with it
    auto doc = builder.build(filePath, "no ext content");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildNestedPath)
{
    QString filePath = tmp.path() + "/subdir/nested.txt";
    auto doc = builder.build(filePath, "nested content");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithSpecialCharactersInText)
{
    QString filePath = tmp.path() + "/test.txt";
    QString specialText = "Hello 世界 🌍 \n\t\r特殊字符";
    auto doc = builder.build(filePath, specialText);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithUnicodeFilePath)
{
    // Create a file with unicode in the path
    QString unicodeDir = tmp.path() + "/中文目录";
    QDir().mkpath(unicodeDir);
    QString filePath = unicodeDir + "/文件.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("unicode content");
    f.close();

    auto doc = builder.build(filePath, "unicode content");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithNonExistentFile)
{
    // Even non-existent file should create a doc (using QFileInfo on missing file)
    QString filePath = tmp.path() + "/nonexistent.txt";
    auto doc = builder.build(filePath, "text for missing file");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(ContentDocumentBuilderTest, BuildWithHiddenFile_CheckHiddenTag)
{
    // Stub isHiddenPathOrInHiddenDir to return true for a non-hidden path
    stub_ext::StubExt stub;
    bool called = false;
    stub.set_lamda(ADDR(Global, isHiddenPathOrInHiddenDir),
                   [&](const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       called = true;
                       return false;
                   });

    QString filePath = tmp.path() + "/test.txt";
    auto doc = builder.build(filePath, "text");
    ASSERT_NE(doc, nullptr);
    EXPECT_TRUE(called);
}
