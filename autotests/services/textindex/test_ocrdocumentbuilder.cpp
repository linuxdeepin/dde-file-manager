// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrdocumentbuilder.cpp
 * @brief Unit tests for OcrDocumentBuilder (document/ocrdocumentbuilder.cpp).
 *        Exercises the build() method with various file and options combinations.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/document/ocrdocumentbuilder.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

class OcrDocumentBuilderTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        QDir dir(tmp.path());
        ASSERT_TRUE(dir.mkpath("subdir"));
        createFile("image.png", "PNG_DATA");
        createFile("subdir/nested.png", "PNG_DATA2");
        createFile("photo.JPG", "JPG_DATA");
    }

    void createFile(const QString &relativePath, const QString &content)
    {
        QFile f(tmp.path() + "/" + relativePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }

    OcrDocumentBuilder builder;
};

TEST_F(OcrDocumentBuilderTest, BuildBasicImageFile)
{
    QString filePath = tmp.path() + "/image.png";
    auto doc = builder.build(filePath, "OCR recognized text");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithEmptyText)
{
    QString filePath = tmp.path() + "/image.png";
    auto doc = builder.build(filePath, "");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithWhitespaceOnlyText)
{
    QString filePath = tmp.path() + "/image.png";
    auto doc = builder.build(filePath, "   \n\t  ");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithLongOcrText)
{
    QString filePath = tmp.path() + "/image.png";
    QString longText(10000, 'X');
    auto doc = builder.build(filePath, longText);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithChecksumInOptions)
{
    QString filePath = tmp.path() + "/image.png";
    BuilderOptions options;
    options.checksum = "md5hash123";
    auto doc = builder.build(filePath, "OCR text", options);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithEmptyChecksumInOptions)
{
    QString filePath = tmp.path() + "/image.png";
    BuilderOptions options;
    options.checksum = "";
    auto doc = builder.build(filePath, "OCR text", options);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithDefaultOptions)
{
    QString filePath = tmp.path() + "/image.png";
    BuilderOptions options;
    auto doc = builder.build(filePath, "OCR text", options);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildFileWithUppercaseExtension)
{
    QString filePath = tmp.path() + "/photo.JPG";
    auto doc = builder.build(filePath, "OCR text for JPG");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildNestedPath)
{
    QString filePath = tmp.path() + "/subdir/nested.png";
    auto doc = builder.build(filePath, "nested OCR");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithSpecialCharacters)
{
    QString filePath = tmp.path() + "/image.png";
    QString specialText = "识别文本 世界 🌍";
    auto doc = builder.build(filePath, specialText);
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildNonExistentFile)
{
    QString filePath = tmp.path() + "/nonexistent.png";
    auto doc = builder.build(filePath, "OCR text for missing file");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithHiddenPath_Stubbed)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(Global, isHiddenPathOrInHiddenDir),
                   [](const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    QString filePath = tmp.path() + "/image.png";
    auto doc = builder.build(filePath, "OCR text");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildFileWithoutExtension)
{
    // Create a file without extension
    QString filePath = tmp.path() + "/noext_file";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    auto doc = builder.build(filePath, "OCR text");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}

TEST_F(OcrDocumentBuilderTest, BuildWithUnicodePath)
{
    QString unicodeDir = tmp.path() + "/中文目录";
    QDir().mkpath(unicodeDir);
    QString filePath = unicodeDir + "/图片.png";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("PNG");
    f.close();

    auto doc = builder.build(filePath, "OCR Unicode content");
    ASSERT_NE(doc, nullptr);
    SUCCEED();
}
