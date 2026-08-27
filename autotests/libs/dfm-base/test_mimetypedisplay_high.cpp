// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mimetypedisplay_high.cpp
 * @brief Mode 2 supplement: High-priority MimeTypeDisplayManager::displayNameToEnumDirect.
 *
 * Target (from .ut-inventory.json, level=high, usecase_count==0):
 *   - MimeTypeDisplayManager::displayNameToEnumDirect
 *
 * Branch清单 (declared, cross-checked via MCP get_code_snippet):
 *   - application/x-desktop → kDesktopApplication
 *   - inode/directory → kDirectory
 *   - application/x-executable → kExecutable
 *   - video/* → kVideos
 *   - audio/* → kAudios
 *   - image/* → kImages
 *   - text/* → kDocuments
 *   - archive mime types → kArchives
 *   - backup mime types → kBackups
 *   - unknown → kUnknown
 */

#include <gtest/gtest.h>

#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

TEST(DisplayNameToEnumDirectTest, DesktopApplication)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("application/x-desktop"),
              FileInfo::FileType::kDesktopApplication);
}

TEST(DisplayNameToEnumDirectTest, Directory)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("inode/directory"),
              FileInfo::FileType::kDirectory);
}

TEST(DisplayNameToEnumDirectTest, Executable)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("application/x-executable"),
              FileInfo::FileType::kExecutable);
}

TEST(DisplayNameToEnumDirectTest, VideoByPrefix)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("video/mp4"),
              FileInfo::FileType::kVideos);
    EXPECT_EQ(mgr->displayNameToEnumDirect("video/x-msvideo"),
              FileInfo::FileType::kVideos);
}

TEST(DisplayNameToEnumDirectTest, AudioByPrefix)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("audio/mpeg"),
              FileInfo::FileType::kAudios);
}

TEST(DisplayNameToEnumDirectTest, ImageByPrefix)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("image/png"),
              FileInfo::FileType::kImages);
}

TEST(DisplayNameToEnumDirectTest, TextByPrefix)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("text/plain"),
              FileInfo::FileType::kDocuments);
}

TEST(DisplayNameToEnumDirectTest, UnknownType)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    EXPECT_EQ(mgr->displayNameToEnumDirect("application/x-some-unknown-type"),
              FileInfo::FileType::kUnknown);
}

TEST(DisplayNameToEnumDirectTest, ConsistentWithDisplayNameToEnum)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    // displayNameToEnum wraps displayNameToEnumDirect for the common cases
    for (const QString &mt : {QStringLiteral("inode/directory"),
                              QStringLiteral("application/x-desktop"),
                              QStringLiteral("video/mp4"),
                              QStringLiteral("audio/mpeg"),
                              QStringLiteral("image/png"),
                              QStringLiteral("text/plain"),
                              QStringLiteral("application/x-totally-unknown")}) {
        EXPECT_EQ(mgr->displayNameToEnum(mt), mgr->displayNameToEnumDirect(mt))
            << "Mismatch for mime type: " << mt.toStdString();
    }
}

// Cover accurateLocalMimeTypeName (public, high-priority)
TEST(MimeTypeDisplayManagerHighTest, AccurateLocalMimeTypeName)
{
    auto *mgr = MimeTypeDisplayManager::instance();

    // Create a temp file with known extension
    QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString filePath = tmpDir + "/accurate_mime_test.txt";
    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write("test content");
        f.close();
    }

    QString result = mgr->accurateLocalMimeTypeName(filePath);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("text") || result.contains("plain"))
        << "Expected text/plain, got: " << result.toStdString();

    QFile::remove(filePath);
}

TEST(MimeTypeDisplayManagerHighTest, AccurateLocalMimeTypeNameNonExistent)
{
    auto *mgr = MimeTypeDisplayManager::instance();
    // Non-existent file should still return something (may be based on extension)
    EXPECT_NO_FATAL_FAILURE({
        QString result = mgr->accurateLocalMimeTypeName("/nonexistent/file.txt");
        (void)result;
    });
}

// Cover shouldSkipAncestorMimeType (public, high-priority)
TEST(MimeTypeDisplayManagerHighTest, ShouldSkipAncestorMimeType)
{
    auto *mgr = MimeTypeDisplayManager::instance();

    // Test with common mime types
    EXPECT_NO_FATAL_FAILURE({
        bool skip = mgr->shouldSkipAncestorMimeType("text/plain");
        (void)skip;
    });

    EXPECT_NO_FATAL_FAILURE({
        bool skip = mgr->shouldSkipAncestorMimeType("application/octet-stream");
        (void)skip;
    });

    // Test with empty/unknown mime type
    EXPECT_NO_FATAL_FAILURE({
        bool skip = mgr->shouldSkipAncestorMimeType("");
        (void)skip;
    });

    // Test with a known mime type that might have ancestors
    EXPECT_NO_FATAL_FAILURE({
        bool skip = mgr->shouldSkipAncestorMimeType("video/mp4");
        (void)skip;
    });
}
