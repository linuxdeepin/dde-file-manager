// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <fstab.h>

#include <QSet>
#include <QMap>
#include <QFileInfo>

#include "utils/indextraverseutils.h"
#include "utils/textindexconfig.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_IndexTraverseUtils : public testing::Test
{
protected:
    void SetUp() override
    {
        // Mock TextIndexConfig
        stub.set_lamda(ADDR(TextIndexConfig, instance), []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(ADDR(TextIndexConfig, supportedFileExtensions), [this](TextIndexConfig *) -> QStringList {
            __DBG_STUB_INVOKE__
            return mockSupportedExtensions;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    QStringList mockSupportedExtensions = { "txt", "pdf", "doc", "docx", "html", "xml" };

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_IndexTraverseUtils, IsHiddenFile_DotFile_ReturnsTrue)
{
    bool result = IndexTraverseUtils::isHiddenFile(".hidden");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexTraverseUtils, IsHiddenFile_DotDirectory_ReturnsTrue)
{
    bool result = IndexTraverseUtils::isHiddenFile(".git");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexTraverseUtils, IsHiddenFile_NormalFile_ReturnsFalse)
{
    bool result = IndexTraverseUtils::isHiddenFile("normal.txt");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsHiddenFile_EmptyString_ReturnsFalse)
{
    bool result = IndexTraverseUtils::isHiddenFile("");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsSpecialDir_CurrentDir_ReturnsTrue)
{
    bool result = IndexTraverseUtils::isSpecialDir(".");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexTraverseUtils, IsSpecialDir_ParentDir_ReturnsTrue)
{
    bool result = IndexTraverseUtils::isSpecialDir("..");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexTraverseUtils, IsSpecialDir_NormalDir_ReturnsFalse)
{
    bool result = IndexTraverseUtils::isSpecialDir("normal");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsSpecialDir_HiddenDir_ReturnsFalse)
{
    bool result = IndexTraverseUtils::isSpecialDir(".hidden");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsValidFile_ValidPath_ReturnsTrue)
{
    // Mock QFileInfo to return valid canonical path
    stub.set_lamda(ADDR(QFileInfo, canonicalFilePath), [](QFileInfo *) -> QString {
        __DBG_STUB_INVOKE__
        return "/valid/path/file.txt";
    });

    bool result = IndexTraverseUtils::isValidFile("/valid/path/file.txt");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexTraverseUtils, IsValidFile_InvalidPath_ReturnsFalse)
{
    // Mock QFileInfo to return empty canonical path
    stub.set_lamda(ADDR(QFileInfo, canonicalFilePath), [](QFileInfo *) -> QString {
        __DBG_STUB_INVOKE__
        return "";
    });

    bool result = IndexTraverseUtils::isValidFile("/invalid/path");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsValidDirectory_ValidNewDirectory_ReturnsTrue)
{
    QSet<QString> visitedDirs;

    // Mock QFileInfo methods
    stub.set_lamda(ADDR(QFileInfo, canonicalFilePath), [](QFileInfo *) -> QString {
        __DBG_STUB_INVOKE__
        return "/valid/directory";
    });

    stub.set_lamda(ADDR(QFileInfo, isSymLink), [](QFileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = IndexTraverseUtils::isValidDirectory("/valid/directory", visitedDirs);

    EXPECT_TRUE(result);
    EXPECT_TRUE(visitedDirs.contains("/valid/directory"));
}

TEST_F(UT_IndexTraverseUtils, IsValidDirectory_SymLink_ReturnsFalse)
{
    QSet<QString> visitedDirs;

    stub.set_lamda(ADDR(QFileInfo, isSymLink), [](QFileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = IndexTraverseUtils::isValidDirectory("/symlink/path", visitedDirs);

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsValidDirectory_EmptyCanonicalPath_ReturnsFalse)
{
    QSet<QString> visitedDirs;

    stub.set_lamda(ADDR(QFileInfo, canonicalFilePath), [](QFileInfo *) -> QString {
        __DBG_STUB_INVOKE__
        return "";
    });

    stub.set_lamda(ADDR(QFileInfo, isSymLink), [](QFileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = IndexTraverseUtils::isValidDirectory("/invalid/path", visitedDirs);

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsValidDirectory_AlreadyVisited_ReturnsFalse)
{
    QSet<QString> visitedDirs;
    visitedDirs.insert("/already/visited");

    stub.set_lamda(ADDR(QFileInfo, canonicalFilePath), [](QFileInfo *) -> QString {
        __DBG_STUB_INVOKE__
        return "/already/visited";
    });

    stub.set_lamda(ADDR(QFileInfo, isSymLink), [](QFileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = IndexTraverseUtils::isValidDirectory("/already/visited", visitedDirs);

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, ShouldSkipDirectory_SystemDirectory_ReturnsTrue)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/proc/self"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/sys/devices"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/dev/null"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/boot/grub"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/tmp/test"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/var/cache/test"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/run/user"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/media/usb"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/mnt/disk"));
}

TEST_F(UT_IndexTraverseUtils, ShouldSkipDirectory_SpecialDirectory_ReturnsTrue)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/opt/software"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/lib/library"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/share/data"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/include/headers"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/src/source"));
}

TEST_F(UT_IndexTraverseUtils, ShouldSkipDirectory_UserDirectory_ReturnsFalse)
{
    EXPECT_FALSE(IndexTraverseUtils::shouldSkipDirectory("/home/user/Documents"));
    EXPECT_FALSE(IndexTraverseUtils::shouldSkipDirectory("/home/user/Desktop"));
    EXPECT_FALSE(IndexTraverseUtils::shouldSkipDirectory("/home/user/Downloads"));
    EXPECT_FALSE(IndexTraverseUtils::shouldSkipDirectory("/custom/path"));
}

TEST_F(UT_IndexTraverseUtils, ExtractFileExtension_ValidFile_ReturnsLowerCaseExtension)
{
    QString result = IndexTraverseUtils::extractFileExtension("Document.TXT");

    EXPECT_EQ(result, "txt");
}

TEST_F(UT_IndexTraverseUtils, ExtractFileExtension_NoExtension_ReturnsEmptyString)
{
    QString result = IndexTraverseUtils::extractFileExtension("README");

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_IndexTraverseUtils, ExtractFileExtension_DotFile_ReturnsEmptyString)
{
    QString result = IndexTraverseUtils::extractFileExtension(".hidden");

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_IndexTraverseUtils, ExtractFileExtension_MultipleExtensions_ReturnsLastExtension)
{
    QString result = IndexTraverseUtils::extractFileExtension("archive.tar.gz");

    EXPECT_EQ(result, "gz");
}

TEST_F(UT_IndexTraverseUtils, ExtractFileExtension_EmptyString_ReturnsEmptyString)
{
    QString result = IndexTraverseUtils::extractFileExtension("");

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_IndexTraverseUtils, IsSupportedFileExtension_SupportedExtension_ReturnsTrue)
{
    bool result = IndexTraverseUtils::isSupportedFileExtension("document.txt");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexTraverseUtils, IsSupportedFileExtension_SupportedExtensionUpperCase_ReturnsTrue)
{
    bool result = IndexTraverseUtils::isSupportedFileExtension("document.PDF");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexTraverseUtils, IsSupportedFileExtension_UnsupportedExtension_ReturnsFalse)
{
    bool result = IndexTraverseUtils::isSupportedFileExtension("binary.exe");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, IsSupportedFileExtension_NoExtension_ReturnsFalse)
{
    bool result = IndexTraverseUtils::isSupportedFileExtension("README");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexTraverseUtils, GetSupportedFileExtensions_ReturnsNonEmptySet)
{
    const QSet<QString> &extensions = IndexTraverseUtils::getSupportedFileExtensions();

    EXPECT_FALSE(extensions.isEmpty());
    EXPECT_TRUE(extensions.contains("txt"));
    EXPECT_TRUE(extensions.contains("pdf"));
    EXPECT_TRUE(extensions.contains("doc"));
}

TEST_F(UT_IndexTraverseUtils, FstabBindInfo_MockedFstab_ReturnsBindMounts)
{
    // Mock fstab functions
    struct MockFstab
    {
        const char *fs_spec;
        const char *fs_file;
        const char *fs_mntops;
    };

    static MockFstab mockEntries[] = {
        { "/dev/sda1", "/", "defaults" },
        { "/home/user/shared", "/mnt/shared", "bind,defaults" },
        { "/tmp/test", "/mnt/test", "bind,ro" },
        { nullptr, nullptr, nullptr }
    };

    static int currentEntry = 0;

    // Mock stat to return valid modification time
    stub.set_lamda(stat, [](const char *path, struct stat *buf) -> int {
        __DBG_STUB_INVOKE__
        buf->st_mtime = 1234567890;
        return 0;
    });

    // Mock fstab functions
    stub.set_lamda(setfsent, []() -> int {
        __DBG_STUB_INVOKE__
        currentEntry = 0;
        return 1;
    });

    stub.set_lamda(
            getfsent, []() -> struct fstab * {
                __DBG_STUB_INVOKE__
                static struct fstab entry;
                if (mockEntries[currentEntry].fs_spec == nullptr) {
                    return nullptr;
                }

                entry.fs_spec = const_cast<char *>(mockEntries[currentEntry].fs_spec);
                entry.fs_file = const_cast<char *>(mockEntries[currentEntry].fs_file);
                entry.fs_mntops = const_cast<char *>(mockEntries[currentEntry].fs_mntops);

                currentEntry++;
                return &entry;
            });

    stub.set_lamda(endfsent, []() -> void {
        __DBG_STUB_INVOKE__
        // Cleanup
    });

    QMap<QString, QString> result = IndexTraverseUtils::fstabBindInfo();

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["/home/user/shared"], "/mnt/shared");
    EXPECT_EQ(result["/tmp/test"], "/mnt/test");
}

TEST_F(UT_IndexTraverseUtils, FstabBindInfo_NoFstabFile_ReturnsEmptyMap)
{
    // Mock stat to return error (file doesn't exist)
    stub.set_lamda(stat, [](const char *path, struct stat *buf) -> int {
        __DBG_STUB_INVOKE__
        return -1;   // Error
    });

    QMap<QString, QString> result = IndexTraverseUtils::fstabBindInfo();

    EXPECT_TRUE(result.isEmpty());
}
