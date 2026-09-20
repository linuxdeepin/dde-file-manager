// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filenamedocumentbuilder.cpp
 * @brief Unit tests for FileNameDocumentBuilder (document/filenamedocumentbuilder.cpp)
 *        Verifies field completeness against the anything schema: file_name (lowercased),
 *        file_name_lower (stored, not-analyzed), full_path (key), file_type, file_ext,
 *        birth/modify/file_size (NumericField), file_size_str (glib format), pinyin/
 *        pinyin_acronym, is_hidden (Y/N), ancestor_paths (multi-value incl. "/").
 *
 * PinyinProcessor is reloaded with a small temp dict so Chinese filename conversion
 * is deterministic; FileTypeMapper returns "other"/"dir" when no dconfig is available.
 *
 * Field verification uses Document::get(name) (returns the stored string value) and
 * Document::getFields(name) (returns the list of matching fields, for multi-value
 * fields and presence checks on numeric fields whose stringValue is empty).
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QString>

#include "stubext.h"
#include <dfm-search/field_names.h>
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/document/filenamedocumentbuilder.h"
#include "services/textindex/utils/pinyinprocessor.h"

#include <lucene++/LuceneHeaders.h>

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;
using namespace Lucene;
using namespace DFMSEARCH::LuceneFieldNames;

namespace {

// Convenience: get the string value of a stored field by name.
// Returns QString() if the field is absent or has no stored string value (e.g. numeric).
QString fieldValue(const DocumentPtr &doc, const QString &name)
{
    return QString::fromStdWString(doc->get(name.toStdWString()));
}

// Count how many fields with the given name exist (for multi-value / numeric fields).
int fieldCount(const DocumentPtr &doc, const QString &name)
{
    return doc->getFields(name.toStdWString()).size();
}

// Reload the pinyin singleton with a minimal dict (文/件/名) so Chinese filenames
// produce deterministic pinyin in document-builder tests.
void loadMiniPinyinDict()
{
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    QTextStream s(&f);
    s << "U+6587: wen  # 文\n";
    s << "U+4EF6: jian  # 件\n";
    s << "U+540D: ming  # 名\n";
    s.flush();
    f.close();
    ASSERT_TRUE(PinyinProcessor::instance().loadDictionary(f.fileName()));
}

}   // namespace

class FileNameDocumentBuilderTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    FileNameDocumentBuilder builder;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        loadMiniPinyinDict();
    }

    void createFile(const QString &relPath, const QString &content = "x")
    {
        QFile f(tmp.path() + "/" + relPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }
};

// --- Basic construction ---

TEST_F(FileNameDocumentBuilderTest, BuildRegularFile_ReturnsNonNullDoc)
{
    createFile("readme.txt");
    auto doc = builder.build(tmp.path() + "/readme.txt", {});
    ASSERT_NE(doc, nullptr);
}

TEST_F(FileNameDocumentBuilderTest, BuildNonExistentFile_ReturnsNonNullDoc)
{
    auto doc = builder.build(tmp.path() + "/nonexistent.txt", {});
    ASSERT_NE(doc, nullptr);   // QFileInfo on missing file yields defaults
}

// --- file_name / file_name_lower ---

TEST_F(FileNameDocumentBuilderTest, FileName_IsLowercased)
{
    createFile("ReadMe.TXT");
    auto doc = builder.build(tmp.path() + "/ReadMe.TXT", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kFileName)), QString("readme.txt"));
}

TEST_F(FileNameDocumentBuilderTest, FileNameLower_IsLowercased)
{
    createFile("MyFile.PDF");
    auto doc = builder.build(tmp.path() + "/MyFile.PDF", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kFileNameLower)), QString("myfile.pdf"));
}

// --- full_path ---

TEST_F(FileNameDocumentBuilderTest, FullPath_ContainsExactFilePath)
{
    createFile("report.doc");
    QString abs = tmp.path() + "/report.doc";
    auto doc = builder.build(abs, {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kFullPath)), abs);
}

// --- file_ext ---

TEST_F(FileNameDocumentBuilderTest, FileExt_IsLowercaseWithoutDot)
{
    createFile("archive.ZIP");
    auto doc = builder.build(tmp.path() + "/archive.ZIP", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kFileExt)), QString("zip"));
}

TEST_F(FileNameDocumentBuilderTest, FileExt_AbsentWhenNoExtension)
{
    createFile("noext");
    auto doc = builder.build(tmp.path() + "/noext", {});
    ASSERT_NE(doc, nullptr);
    // No extension → file_ext field should not be present
    EXPECT_TRUE(fieldValue(doc, QString::fromWCharArray(FileName::kFileExt)).isEmpty());
}

// --- file_type ---

TEST_F(FileNameDocumentBuilderTest, FileType_DirectoryIsDir)
{
    auto doc = builder.build(tmp.path(), {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kFileType)), QString("dir"));
}

TEST_F(FileNameDocumentBuilderTest, FileType_RegularFileIsOtherWithoutDConfig)
{
    createFile("data.bin");
    auto doc = builder.build(tmp.path() + "/data.bin", {});
    ASSERT_NE(doc, nullptr);
    // Without a real dconfig, unknown extension → "other"
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kFileType)), QString("other"));
}

// --- is_hidden ---

TEST_F(FileNameDocumentBuilderTest, IsHidden_NonHiddenFileIsN)
{
    createFile("visible.txt");
    auto doc = builder.build(tmp.path() + "/visible.txt", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kIsHidden)), QString("N"));
}

TEST_F(FileNameDocumentBuilderTest, IsHidden_HiddenFileIsY)
{
    createFile(".secret.txt");
    auto doc = builder.build(tmp.path() + "/.secret.txt", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kIsHidden)), QString("Y"));
}

TEST_F(FileNameDocumentBuilderTest, IsHidden_FileInHiddenDirectoryIsY)
{
    QDir d(tmp.path());
    ASSERT_TRUE(d.mkpath(".hiddenDir"));
    createFile(".hiddenDir/note.txt");
    auto doc = builder.build(tmp.path() + "/.hiddenDir/note.txt", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kIsHidden)), QString("Y"));
}

// --- ancestor_paths ---

TEST_F(FileNameDocumentBuilderTest, AncestorPaths_MultiValueInclusiveOfRoot)
{
    QDir d(tmp.path());
    ASSERT_TRUE(d.mkpath("sub"));
    createFile("sub/file.txt");
    QString abs = tmp.path() + "/sub/file.txt";
    auto doc = builder.build(abs, {});
    ASSERT_NE(doc, nullptr);
    // At least the parent dir and root "/" should be present (anything adds "/" too)
    EXPECT_GE(fieldCount(doc, QString::fromWCharArray(FileName::kAncestorPaths)), 2);
}

TEST_F(FileNameDocumentBuilderTest, AncestorPaths_FileInRootDepthOne)
{
    createFile("root.txt");
    QString abs = tmp.path() + "/root.txt";
    auto doc = builder.build(abs, {});
    ASSERT_NE(doc, nullptr);
    EXPECT_GE(fieldCount(doc, QString::fromWCharArray(FileName::kAncestorPaths)), 1);
}

// --- pinyin / pinyin_acronym ---

TEST_F(FileNameDocumentBuilderTest, Pinyin_ChineseFilename_ProducedFromDict)
{
    // 文件名.txt → pinyin "wenjianming.txt" / acronym "wjm.txt"
    // (.txt is non-dict chars, passed through — matching anything behavior)
    createFile("文件名.txt");
    auto doc = builder.build(tmp.path() + "/文件名.txt", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kPinyin)), QString("wenjianming.txt"));
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kPinyinAcronym)), QString("wjm.txt"));
}

TEST_F(FileNameDocumentBuilderTest, Pinyin_PureChineseFilename_OnlyPinyin)
{
    // 文件名 (no extension) → pure pinyin "wenjianming" / "wjm"
    QDir d(tmp.path());
    QFile f(tmp.path() + "/文件名");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();
    auto doc = builder.build(tmp.path() + "/文件名", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kPinyin)), QString("wenjianming"));
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kPinyinAcronym)), QString("wjm"));
}

TEST_F(FileNameDocumentBuilderTest, Pinyin_NonChineseFilename_PassedThroughLowercased)
{
    // Pure ASCII filename: pinyin field equals the lowercased filename (non-dict passthrough)
    createFile("ReadMe.TXT");
    auto doc = builder.build(tmp.path() + "/ReadMe.TXT", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kPinyin)), QString("readme.txt"));
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kPinyinAcronym)), QString("readme.txt"));
}

// --- file_size / file_size_str ---

TEST_F(FileNameDocumentBuilderTest, FileSize_NumericFieldPresent)
{
    createFile("sized.bin", "12345678");
    auto doc = builder.build(tmp.path() + "/sized.bin", {});
    ASSERT_NE(doc, nullptr);
    // file_size is a NumericField — get() returns empty string, but the field is present.
    EXPECT_GE(fieldCount(doc, QString::fromWCharArray(FileName::kFileSize)), 1);
}

TEST_F(FileNameDocumentBuilderTest, FileSizeStr_FormattedHumanReadable)
{
    createFile("sized.bin", "12345678");   // ~12 MB after 1000-based formatting
    auto doc = builder.build(tmp.path() + "/sized.bin", {});
    ASSERT_NE(doc, nullptr);
    QString sizeStr = fieldValue(doc, QString::fromWCharArray(FileName::kFileSizeStr));
    EXPECT_FALSE(sizeStr.isEmpty());
    // g_format_size produces localized strings; verify non-empty & contains a digit
    bool hasDigit = false;
    for (const QChar &c : sizeStr) {
        if (c.isDigit()) { hasDigit = true; break; }
    }
    EXPECT_TRUE(hasDigit);
}

// --- timestamp fields ---

TEST_F(FileNameDocumentBuilderTest, BirthAndModifyTime_NumericFieldsPresent)
{
    createFile("timed.txt");
    auto doc = builder.build(tmp.path() + "/timed.txt", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_GE(fieldCount(doc, QString::fromWCharArray(FileName::kBirthTime)), 1);
    EXPECT_GE(fieldCount(doc, QString::fromWCharArray(FileName::kModifyTime)), 1);
}

// --- builder options / text argument are ignored (filename has no content) ---

TEST_F(FileNameDocumentBuilderTest, Build_IgnoresTextArgument)
{
    createFile("a.txt");
    auto doc1 = builder.build(tmp.path() + "/a.txt", QString());
    auto doc2 = builder.build(tmp.path() + "/a.txt", QStringLiteral("ignored content"));
    ASSERT_NE(doc1, nullptr);
    ASSERT_NE(doc2, nullptr);
    // file_name field should be identical regardless of text arg
    EXPECT_EQ(fieldValue(doc1, QString::fromWCharArray(FileName::kFileName)),
              fieldValue(doc2, QString::fromWCharArray(FileName::kFileName)));
}

// --- unicode path ---

TEST_F(FileNameDocumentBuilderTest, Build_UnicodePathNoCrash)
{
    QDir d(tmp.path());
    ASSERT_TRUE(d.mkpath("中文目录"));
    QFile f(tmp.path() + "/中文目录/文件.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    auto doc = builder.build(tmp.path() + "/中文目录/文件.txt", {});
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(fieldValue(doc, QString::fromWCharArray(FileName::kFileName)), QString("文件.txt"));
}
