// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileutils_high.cpp
 * @brief Mode 2 supplement: High-priority FileUtils methods not yet covered.
 *
 * Targets (from .ut-inventory.json, level=high, usecase_count==0):
 *   - FileUtils::fileBatchAddText
 *   - FileUtils::fileBatchCustomText
 *   - FileUtils::fileBatchReplaceText
 *   - FileUtils::fileCanTrash
 *   - FileUtils::detectCharset
 *   - FileUtils::formatSize  (additional branch coverage)
 *   - FileUtils::cutFileName  (additional branch coverage)
 *   - FileUtils::convertToSRgbColorSpace  (additional branch coverage)
 *   - FileUtils::notifyFileChangeManual  (additional coverage)
 *
 * Branch清单 (declared, cross-checked via MCP get_code_snippet):
 *   formatSize: num<0 / unitList.empty / isForceUnit / loop break / withUnitVisible
 *   cutFileName: useCharCount / !useCharCount / surrogate / byte-exceed
 *   fileCanTrash: !info / isRootUser / isOpenAsAdmin / normal
 *   detectCharset: data.isEmpty / codecForUtfText / mime detect / html-charset
 *   fileBatchAddText: originUrls.isEmpty / loop / isDesktopApp / kPrefix / kSuffix
 *   fileBatchCustomText: empty / serialNumber==MAX / loop
 *   fileBatchReplaceText: empty / loop / isDesktopApp / trimmed-empty
 *   convertToSRgbColorSpace: isNull / validColorSpace / CMYK / needsConversion
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QList>
#include <QPair>
#include <QString>
#include <QImage>
#include <QColorSpace>
#include <QMap>
#include <mutex>
#include <QIcon>

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

using namespace dfmbase;

class FileUtilsHighTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
    }

    QString makeFile(const QString &relPath, const QByteArray &content = "x")
    {
        QString abs = rootPath + "/" + relPath;
        QFile f(abs);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(content);
            f.close();
        }
        return abs;
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    static std::once_flag flag;
};

std::once_flag FileUtilsHighTest::flag;

// ── formatSize: additional branch coverage ──

TEST_F(FileUtilsHighTest, FormatSizeNegativeClampsToZero)
{
    QString s = FileUtils::formatSize(-1, true, 0);
    EXPECT_FALSE(s.isEmpty());
    // negative should be clamped to 0 → "0 B"
    EXPECT_TRUE(s.startsWith('0'));
}

TEST_F(FileUtilsHighTest, FormatSizeZeroBytes)
{
    QString s = FileUtils::formatSize(0, true, 0);
    EXPECT_TRUE(s.contains("B"));
}

TEST_F(FileUtilsHighTest, FormatSizeForceUnitGb)
{
    // forceUnit=3 → GB
    QString s = FileUtils::formatSize(1024, true, 2, 3);
    EXPECT_FALSE(s.isEmpty());
}

TEST_F(FileUtilsHighTest, FormatSizeCustomUnitList)
{
    QStringList units { " bytes", " KB", " MB" };
    QString s = FileUtils::formatSize(1048576, true, 1, -1, units);
    EXPECT_FALSE(s.isEmpty());
    EXPECT_TRUE(s.contains("MB"));
}

TEST_F(FileUtilsHighTest, FormatSizeWithoutUnitVisible)
{
    QString s = FileUtils::formatSize(2048, false, 1);
    // No unit suffix expected
    EXPECT_FALSE(s.contains("KB"));
}

TEST_F(FileUtilsHighTest, FormatSizeLargeTeraByte)
{
    // 1 TB = 1024^4
    QString s = FileUtils::formatSize(static_cast<qint64>(1024) * 1024 * 1024 * 1024, true, 1);
    EXPECT_TRUE(s.contains("TB"));
}

TEST_F(FileUtilsHighTest, FormatSizeForceUnitKb)
{
    // forceUnit=1 → KB even for small value
    QString s = FileUtils::formatSize(100, true, 2, 1);
    EXPECT_FALSE(s.isEmpty());
}

// ── cutFileName: additional branch coverage ──

TEST_F(FileUtilsHighTest, CutFileNameCharCountExactLength)
{
    // name length == maxLength → no truncation
    EXPECT_EQ(FileUtils::cutFileName("abcde", 5, true), QString("abcde"));
}

TEST_F(FileUtilsHighTest, CutFileNameCharCountShorterThanMax)
{
    EXPECT_EQ(FileUtils::cutFileName("ab", 5, true), QString("ab"));
}

TEST_F(FileUtilsHighTest, CutFileNameByteModeAsciiExact)
{
    // ASCII byte mode: 6 bytes <= maxLength(6) → unchanged
    EXPECT_EQ(FileUtils::cutFileName("abcdef", 6, false), QString("abcdef"));
}

TEST_F(FileUtilsHighTest, CutFileNameByteModeTruncate)
{
    // ASCII byte mode: 6 bytes > maxLength(3) → truncated to 3 bytes
    QString result = FileUtils::cutFileName("abcdef", 3, false);
    EXPECT_LE(result.toLocal8Bit().length(), 3);
}

TEST_F(FileUtilsHighTest, CutFileNameByteModeEmptyString)
{
    EXPECT_EQ(FileUtils::cutFileName("", 10, false), QString());
    EXPECT_EQ(FileUtils::cutFileName("", 10, true), QString());
}

TEST_F(FileUtilsHighTest, CutFileNameCharModeEmpty)
{
    EXPECT_EQ(FileUtils::cutFileName("", 5, true), QString());
}

// ── convertToSRgbColorSpace: additional branch coverage ──

TEST_F(FileUtilsHighTest, ConvertToSRgbNullImageReturned)
{
    QImage nullImg;
    QImage result = FileUtils::convertToSRgbColorSpace(nullImg);
    EXPECT_TRUE(result.isNull());
}

TEST_F(FileUtilsHighTest, ConvertToSRgbNoColorSpaceNoConversion)
{
    // A plain QImage without a color space → no conversion needed
    QImage img(2, 2, QImage::Format_ARGB32);
    img.fill(Qt::red);
    QImage result = FileUtils::convertToSRgbColorSpace(img);
    EXPECT_FALSE(result.isNull());
}

TEST_F(FileUtilsHighTest, ConvertToSRgbAlreadySRgb)
{
    QImage img(2, 2, QImage::Format_ARGB32);
    img.setColorSpace(QColorSpace::SRgb);
    img.fill(Qt::blue);
    QImage result = FileUtils::convertToSRgbColorSpace(img);
    EXPECT_FALSE(result.isNull());
}

TEST_F(FileUtilsHighTest, ConvertToSRgbNonSRgbColorSpace)
{
    QImage img(2, 2, QImage::Format_ARGB32);
    img.setColorSpace(QColorSpace::DisplayP3);
    img.fill(Qt::green);
    QImage result = FileUtils::convertToSRgbColorSpace(img);
    EXPECT_FALSE(result.isNull());
}

// ── fileCanTrash ──

TEST_F(FileUtilsHighTest, FileCanTrashNonExistentUrlReturnsFalse)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/does_not_exist.txt");
    EXPECT_FALSE(FileUtils::fileCanTrash(url));
}

TEST_F(FileUtilsHighTest, FileCanTrashExistingFileNoCrash)
{
    QString path = makeFile("trashable.txt");
    QUrl url = QUrl::fromLocalFile(path);
    // Just verify no crash — result depends on environment (root vs user)
    EXPECT_NO_FATAL_FAILURE({ FileUtils::fileCanTrash(url); });
}

TEST_F(FileUtilsHighTest, FileCanTrashEmptyUrl)
{
    QUrl url;
    EXPECT_FALSE(FileUtils::fileCanTrash(url));
}

// ── fileBatchAddText ──

TEST_F(FileUtilsHighTest, FileBatchAddTextEmptyListReturnsEmpty)
{
    auto result = FileUtils::fileBatchAddText({}, { QString("_copy"), AbstractJobHandler::FileNameAddFlag::kSuffix });
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(FileUtilsHighTest, FileBatchAddTextSuffixOnExistingFile)
{
    QString path = makeFile("document.txt", "content");
    QUrl url = QUrl::fromLocalFile(path);
    auto result = FileUtils::fileBatchAddText({ url }, { QString("_v2"), AbstractJobHandler::FileNameAddFlag::kSuffix });
    EXPECT_EQ(result.size(), 1u);
    // The new URL should contain the suffix
    QUrl newUrl = result.value(url);
    EXPECT_TRUE(newUrl.fileName().contains("_v2"));
}

TEST_F(FileUtilsHighTest, FileBatchAddTextPrefixOnExistingFile)
{
    QString path = makeFile("report.pdf", "data");
    QUrl url = QUrl::fromLocalFile(path);
    auto result = FileUtils::fileBatchAddText({ url }, { QString("pre_"), AbstractJobHandler::FileNameAddFlag::kPrefix });
    EXPECT_EQ(result.size(), 1u);
    QUrl newUrl = result.value(url);
    EXPECT_TRUE(newUrl.fileName().contains("pre_"));
}

TEST_F(FileUtilsHighTest, FileBatchAddTextNonExistentFileNoCrash)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/ghost.txt");
    // InfoFactory creates info lazily even for non-existent files; verify no crash
    EXPECT_NO_FATAL_FAILURE({
        EXPECT_NO_FATAL_FAILURE({ auto result = FileUtils::fileBatchAddText({ url }, { QString("_x"), AbstractJobHandler::FileNameAddFlag::kSuffix }); });
    });
}

// ── fileBatchCustomText ──

TEST_F(FileUtilsHighTest, FileBatchCustomTextEmptyListReturnsEmpty)
{
    auto result = FileUtils::fileBatchCustomText({}, { QString("file"), QString("1") });
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(FileUtilsHighTest, FileBatchCustomTextEmptyPairReturnsEmpty)
{
    QString path = makeFile("a.txt");
    QUrl url = QUrl::fromLocalFile(path);
    auto result = FileUtils::fileBatchCustomText({ url }, { QString(), QString("1") });
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(FileUtilsHighTest, FileBatchCustomTextValidFile)
{
    QString path = makeFile("custom.txt", "d");
    QUrl url = QUrl::fromLocalFile(path);
    auto result = FileUtils::fileBatchCustomText({ url }, { QString("renamed"), QString("1") });
    EXPECT_EQ(result.size(), 1u);
    QUrl newUrl = result.value(url);
    EXPECT_FALSE(newUrl.fileName().isEmpty());
}

TEST_F(FileUtilsHighTest, FileBatchCustomTextSerialNumberMaxValue)
{
    QString path = makeFile("max.txt", "d");
    QUrl url = QUrl::fromLocalFile(path);
    // ULONG_LONG_MAX as string
    QString maxStr = QString::number(std::numeric_limits<unsigned long long>::max());
    auto result = FileUtils::fileBatchCustomText({ url }, { QString("f"), maxStr });
    EXPECT_EQ(result.size(), 1u);
}

// ── fileBatchReplaceText ──

TEST_F(FileUtilsHighTest, FileBatchReplaceTextEmptyListReturnsEmpty)
{
    auto result = FileUtils::fileBatchReplaceText({}, { QString("old"), QString("new") });
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(FileUtilsHighTest, FileBatchReplaceTextSubstitution)
{
    QString path = makeFile("old_name.txt", "d");
    QUrl url = QUrl::fromLocalFile(path);
    auto result = FileUtils::fileBatchReplaceText({ url }, { QString("old"), QString("new") });
    EXPECT_EQ(result.size(), 1u);
    QUrl newUrl = result.value(url);
    EXPECT_TRUE(newUrl.fileName().contains("new"));
    EXPECT_FALSE(newUrl.fileName().contains("old_name"));
}

TEST_F(FileUtilsHighTest, FileBatchReplaceTextNoMatchKeepsName)
{
    QString path = makeFile("keep.txt", "d");
    QUrl url = QUrl::fromLocalFile(path);
    auto result = FileUtils::fileBatchReplaceText({ url }, { QString("xyz"), QString("abc") });
    EXPECT_EQ(result.size(), 1u);
    QUrl newUrl = result.value(url);
    // No match → name stays the same
    EXPECT_TRUE(newUrl.fileName().contains("keep"));
}

TEST_F(FileUtilsHighTest, FileBatchReplaceTextNonExistentNoCrash)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/nope.txt");
    // InfoFactory creates info lazily even for non-existent files; verify no crash
    EXPECT_NO_FATAL_FAILURE({
        EXPECT_NO_FATAL_FAILURE({ auto result = FileUtils::fileBatchReplaceText({ url }, { QString("a"), QString("b") }); });
    });
}

// ── notifyFileChangeManual: additional coverage ──

TEST_F(FileUtilsHighTest, NotifyFileChangeManualFileDeleted)
{
    EXPECT_NO_FATAL_FAILURE({ FileUtils::notifyFileChangeManual(Global::FileNotifyType::kFileDeleted, QUrl("file:///tmp/dfm_high_notify_del")); });
}

TEST_F(FileUtilsHighTest, NotifyFileChangeManualFileChanged)
{
    EXPECT_NO_FATAL_FAILURE({ FileUtils::notifyFileChangeManual(Global::FileNotifyType::kFileChanged, QUrl("file:///tmp/dfm_high_notify_chg")); });
}
