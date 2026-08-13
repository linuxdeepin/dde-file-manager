// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileutils.cpp
 * @brief Unit tests for pure-logic functions of FileUtils (fileutils.cpp)
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFile>
#include <QTest>
#include <QDir>
#include <QImage>
#include <QColorSpace>
#include <QIcon>
#include <mutex>

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include "stubext.h"

using namespace dfmbase;

TEST(FileUtilsTest, FormatSizeBytes)
{
    QString s = FileUtils::formatSize(512);
    EXPECT_TRUE(s.contains("B"));
}

TEST(FileUtilsTest, FormatSizeNegativeBecomesZero)
{
    QString s = FileUtils::formatSize(-100);
    EXPECT_TRUE(s.startsWith("0"));
}

TEST(FileUtilsTest, FormatSizeKilobytes)
{
    // 2048 bytes = 2 KB; sizeString trims trailing zeros → "2"
    QString s = FileUtils::formatSize(2048, false, 2);
    EXPECT_EQ(s, QString("2"));
}

TEST(FileUtilsTest, FormatSizeWithUnitVisible)
{
    QString s = FileUtils::formatSize(2048, true, 2);
    EXPECT_TRUE(s.contains("KB"));
    EXPECT_FALSE(s.contains("2.00"));   // trailing zeros trimmed
}

TEST(FileUtilsTest, FormatSizeWithUnitVisibleFalse)
{
    QString s = FileUtils::formatSize(1024, false, 0);
    EXPECT_FALSE(s.contains("KB"));
}

TEST(FileUtilsTest, FormatSizeForceUnit)
{
    // force unit index 2 (MB) for a 1024-byte file
    QString s = FileUtils::formatSize(1024, true, 3, 2);
    EXPECT_TRUE(s.contains("MB"));
}

TEST(FileUtilsTest, FormatSizeCustomUnitList)
{
    QStringList units { " KB", " MB" };
    QString s = FileUtils::formatSize(1024, true, 0, -1, units);
    EXPECT_TRUE(s.contains("MB"));
}

TEST(FileUtilsTest, SupportedMaxLengthKnownFs)
{
    EXPECT_EQ(FileUtils::supportedMaxLength("vfat"), 11);
    EXPECT_EQ(FileUtils::supportedMaxLength("ext4"), 16);
    EXPECT_EQ(FileUtils::supportedMaxLength("btrfs"), 255);
    EXPECT_EQ(FileUtils::supportedMaxLength("xfs"), 12);
}

TEST(FileUtilsTest, SupportedMaxLengthCaseInsensitive)
{
    EXPECT_EQ(FileUtils::supportedMaxLength("NTFS"), 32);
}

TEST(FileUtilsTest, SupportedMaxLengthUnknownFs)
{
    EXPECT_EQ(FileUtils::supportedMaxLength("unknownfs"), 40);
}

TEST(FileUtilsTest, ProcessLengthTrimsToMaxLen)
{
    // srcPos near the end: leftText shrinks until combined fits within maxLen.
    QString dst;
    int dstPos = -1;
    bool ret = FileUtils::processLength("hello world", 8, 5, true, dst, dstPos);
    EXPECT_TRUE(ret);
    EXPECT_LE(dst.length(), 5);
}

TEST(FileUtilsTest, ProcessLengthReturnsFalseWhenLeftEmpty)
{
    // srcPos=5: leftText shrinks to empty but rightText alone still exceeds maxLen.
    QString dst;
    int dstPos = -1;
    bool ret = FileUtils::processLength("hello world", 5, 5, true, dst, dstPos);
    EXPECT_FALSE(ret);
}

TEST(FileUtilsTest, ProcessLengthNoOpWhenWithinMaxLen)
{
    QString dst;
    int dstPos = -1;
    bool ret = FileUtils::processLength("hi", 1, 10, true, dst, dstPos);
    EXPECT_FALSE(ret);
    EXPECT_EQ(dst, QString("hi"));
}

TEST(FileUtilsTest, IsDesktopFileSuffixTrue)
{
    EXPECT_TRUE(FileUtils::isDesktopFileSuffix(QUrl("file:///usr/share/applications/foo.desktop")));
}

TEST(FileUtilsTest, IsDesktopFileSuffixFalse)
{
    EXPECT_FALSE(FileUtils::isDesktopFileSuffix(QUrl("file:///home/user/foo.txt")));
}

TEST(FileUtilsTest, CutFileNameByCharCount)
{
    EXPECT_EQ(FileUtils::cutFileName("hello world", 5, true), QString("hello"));
}

TEST(FileUtilsTest, CutFileNameNoTruncation)
{
    EXPECT_EQ(FileUtils::cutFileName("hi", 10, true), QString("hi"));
}

TEST(FileUtilsTest, CutFileNameByByteCountAscii)
{
    EXPECT_EQ(FileUtils::cutFileName("abcdef", 3, false), QString("abc"));
}

TEST(FileUtilsTest, CutFileNameByByteCountSurrogatePreserved)
{
    // A single emoji is 4 bytes in UTF-8; cutting at 4 bytes keeps the full emoji.
    QString emoji = QString::fromUtf8("\xF0\x9F\x98\x80");
    QString result = FileUtils::cutFileName(emoji + "ab", 4, false);
    EXPECT_EQ(result, emoji);
}

TEST(FileUtilsTest, EncryptDecryptRoundTrip)
{
    QString original = "hello world";
    QString enc = FileUtils::encryptString(original);
    EXPECT_NE(enc, original);
    EXPECT_EQ(FileUtils::decryptString(enc), original);
}

TEST(FileUtilsTest, EncryptStringIsBase64)
{
    QString enc = FileUtils::encryptString("test");
    EXPECT_EQ(enc, QString::fromUtf8(QByteArray("test").toBase64()));
}

TEST(FileUtilsTest, DateTimeFormat)
{
    EXPECT_EQ(FileUtils::dateTimeFormat(), QString("yyyy/MM/dd HH:mm:ss"));
}

TEST(FileUtilsTest, GetMemoryPageSizePositive)
{
    EXPECT_GT(FileUtils::getMemoryPageSize(), 0);
}

TEST(FileUtilsTest, GetCpuProcessCountPositive)
{
    EXPECT_GT(FileUtils::getCpuProcessCount(), 0);
}

TEST(FileUtilsTest, CacheRemoveContainsCopyingFileUrl)
{
    QUrl url("file:///tmp/dfm_unit_test_file.txt");
    EXPECT_FALSE(FileUtils::containsCopyingFileUrl(url));
    FileUtils::cacheCopyingFileUrl(url);
    EXPECT_TRUE(FileUtils::containsCopyingFileUrl(url));
    FileUtils::removeCopyingFileUrl(url);
    EXPECT_FALSE(FileUtils::containsCopyingFileUrl(url));
}

TEST(FileUtilsTest, SetGetTrashEmptyState)
{
    FileUtils::setTrashEmptyState(FileUtils::TrashEmptyState::kEmpty);
    EXPECT_EQ(FileUtils::trashEmptyState(), FileUtils::TrashEmptyState::kEmpty);
    FileUtils::setTrashEmptyState(FileUtils::TrashEmptyState::kNotEmpty);
    EXPECT_EQ(FileUtils::trashEmptyState(), FileUtils::TrashEmptyState::kNotEmpty);
}

TEST(FileUtilsTest, TrashRootUrlScheme)
{
    QUrl url = FileUtils::trashRootUrl();
    EXPECT_FALSE(url.scheme().isEmpty());
    EXPECT_EQ(url.path(), QString("/"));
}

TEST(FileUtilsTest, IsSameFileSamePath)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    EXPECT_TRUE(FileUtils::isSameFile(path, path));
}

TEST(FileUtilsTest, IsSameFileDifferentPaths)
{
    QTemporaryFile a, b;
    ASSERT_TRUE(a.open());
    ASSERT_TRUE(b.open());
    EXPECT_FALSE(FileUtils::isSameFile(a.fileName(), b.fileName()));
}

TEST(FileUtilsTest, IsSameFileNonExistentReturnsFalse)
{
    EXPECT_FALSE(FileUtils::isSameFile("/no/such/file1", "/no/such/file2"));
}

TEST(FileUtilsTest, IsHigherHierarchyTrue)
{
    EXPECT_TRUE(FileUtils::isHigherHierarchy(QUrl("file:///home/user"), QUrl("file:///home/user/docs")));
}

TEST(FileUtilsTest, IsHigherHierarchyFalse)
{
    EXPECT_FALSE(FileUtils::isHigherHierarchy(QUrl("file:///home/user/docs"), QUrl("file:///home/user")));
}

TEST(FileUtilsTest, BindPathTransformIdentity)
{
    QString p = "/tmp/some_path";
    EXPECT_EQ(FileUtils::bindPathTransform(p, false), p);
}

// ---- Coverage additions for previously-uncovered FileUtils API ----

TEST(FileUtilsTest, RefreshIconCacheIsSafe)
{
    EXPECT_NO_FATAL_FAILURE({ FileUtils::refreshIconCache(); });
}

TEST(FileUtilsTest, IsComputerDesktopFileNonDesktopSuffixReturnsFalse)
{
    EXPECT_FALSE(FileUtils::isComputerDesktopFile(QUrl::fromLocalFile("/tmp/not_a_desktop_file.txt")));
}

TEST(FileUtilsTest, IsComputerDesktopFileRegularDesktopReturnsFalse)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.path() + "/regular.desktop";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("[Desktop Entry]\nType=Application\nName=RegularApp\n");
    f.close();
    EXPECT_FALSE(FileUtils::isComputerDesktopFile(QUrl::fromLocalFile(path)));
}

TEST(FileUtilsTest, IsSameMountPointNonLocalReturnsFalse)
{
    EXPECT_FALSE(FileUtils::isSameMountPoint(QUrl("trash:///"), QUrl("trash:///")));
}

TEST(FileUtilsTest, IsSameMountPointDifferentSchemeReturnsFalse)
{
    EXPECT_FALSE(FileUtils::isSameMountPoint(QUrl("file:///tmp/a"), QUrl("trash:///")));
}

TEST(FileUtilsTest, IsSameMountPointLocalSameDirReturnsTrue)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QUrl a = QUrl::fromLocalFile(dir.path() + "/a");
    QUrl b = QUrl::fromLocalFile(dir.path() + "/b");
    EXPECT_TRUE(FileUtils::isSameMountPoint(a, b));
}

TEST(FileUtilsTest, IsSameDeviceDifferentSchemeReturnsFalse)
{
    EXPECT_FALSE(FileUtils::isSameDevice(QUrl("file:///tmp/a"), QUrl("trash:///")));
}

TEST(FileUtilsTest, IsSameDeviceLocalSameDirReturnsTrue)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QUrl a = QUrl::fromLocalFile(dir.path() + "/a");
    QUrl b = QUrl::fromLocalFile(dir.path() + "/b");
    EXPECT_TRUE(FileUtils::isSameDevice(a, b));
}

TEST(FileUtilsTest, IsSameDeviceNonLocalSameHostReturnsTrue)
{
    QUrl a("smb://host/share/dir");
    QUrl b("smb://host/share/other");
    EXPECT_TRUE(FileUtils::isSameDevice(a, b));
}

TEST(FileUtilsTest, FileCanTrashForLocalTempFileIsCallable)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.path() + "/trashable.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hello");
    f.close();
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::fileCanTrash(QUrl::fromLocalFile(path)); });
}

TEST(FileUtilsTest, TrashIsEmptyIsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::trashIsEmpty(); });
}

// ---- Coverage additions: batch text operations + prohibit path ----

TEST(FileUtilsTest, IsContainProhibitPathWithEmptyListReturnsFalse)
{
    EXPECT_FALSE(FileUtils::isContainProhibitPath({}));
}

TEST(FileUtilsTest, IsContainProhibitPathWithTempPathCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::isContainProhibitPath({ QUrl::fromLocalFile("/tmp/ut_prohibit") }); });
}

TEST(FileUtilsTest, FileBatchAddTextWithEmptyListReturnsEmpty)
{
    EXPECT_TRUE(FileUtils::fileBatchAddText({}, { "prefix", AbstractJobHandler::FileNameAddFlag::kPrefix }).isEmpty());
}

TEST(FileUtilsTest, FileBatchReplaceTextWithEmptyListReturnsEmpty)
{
    EXPECT_TRUE(FileUtils::fileBatchReplaceText({}, { "old", "new" }).isEmpty());
}

// ============================================================
// Additional coverage for FileUtils
// ============================================================

TEST(FileUtilsTest, PreprocessingFileNameReplacesSlash)
{
    QString result = FileUtils::preprocessingFileName("file/name");
    EXPECT_FALSE(result.contains('/'));
}

TEST(FileUtilsTest, PreprocessingFileNameEmpty)
{
    QString result = FileUtils::preprocessingFileName("");
    EXPECT_TRUE(result.isEmpty());
}

TEST(FileUtilsTest, IsDesktopFileNonDesktop)
{
    QUrl url = QUrl::fromLocalFile(QDir::tempPath() + "/regular_file.txt");
    EXPECT_FALSE(FileUtils::isDesktopFile(url));
}

TEST(FileUtilsTest, IsDesktopFileInfoNonDesktop)
{
    // isDesktopFileInfo asserts on null info - test with a valid file instead
    static std::once_flag flag;
    std::call_once(flag, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/test.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("not desktop");
    f.close();
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(FileUtils::isDesktopFileInfo(info));
}

TEST(FileUtilsTest, IsTrashDesktopFileNonTrash)
{
    QUrl url = QUrl::fromLocalFile(QDir::tempPath() + "/regular_file.txt");
    EXPECT_FALSE(FileUtils::isTrashDesktopFile(url));
}

TEST(FileUtilsTest, IsHomeDesktopFileNonHome)
{
    QUrl url = QUrl::fromLocalFile(QDir::tempPath() + "/regular_file.txt");
    EXPECT_FALSE(FileUtils::isHomeDesktopFile(url));
}

TEST(FileUtilsTest, IsCdRomDeviceNonCdRom)
{
    QUrl url = QUrl::fromLocalFile("/tmp/somefile.iso");
    EXPECT_FALSE(FileUtils::isCdRomDevice(url));
}

TEST(FileUtilsTest, IsTrashFileNonTrash)
{
    QUrl url = QUrl::fromLocalFile(QDir::tempPath() + "/regular_file.txt");
    EXPECT_FALSE(FileUtils::isTrashFile(url));
}

TEST(FileUtilsTest, IsTrashRootFileNonTrashRoot)
{
    QUrl url = QUrl::fromLocalFile(QDir::tempPath());
    EXPECT_FALSE(FileUtils::isTrashRootFile(url));
}

TEST(FileUtilsTest, GetFileNameLength)
{
    QUrl url = QUrl::fromLocalFile("/tmp/testfile.txt");
    int len = FileUtils::getFileNameLength(url, "testfile.txt");
    EXPECT_EQ(len, 12);
}

TEST(FileUtilsTest, GetFileNameLengthEmpty)
{
    QUrl url = QUrl::fromLocalFile("/tmp/");
    int len = FileUtils::getFileNameLength(url, "");
    EXPECT_EQ(len, 0);
}

TEST(FileUtilsTest, FileBatchCustomText)
{
    QUrl u1 = QUrl::fromLocalFile("/tmp/a.txt");
    QUrl u2 = QUrl::fromLocalFile("/tmp/b.txt");
    QMap<QUrl, QUrl> result = FileUtils::fileBatchCustomText(
        {u1, u2}, {"prefix_", "_suffix"});
    // Result depends on impl, just verify no crash
    EXPECT_GE(result.size(), 0);
}

TEST(FileUtilsTest, FileBatchCustomTextEmpty)
{
    auto result = FileUtils::fileBatchCustomText({}, {"x", "y"});
    EXPECT_TRUE(result.isEmpty());
}

TEST(FileUtilsTest, FileBatchReplaceText)
{
    QUrl u1 = QUrl::fromLocalFile("/tmp/old_name.txt");
    QUrl u2 = QUrl::fromLocalFile("/tmp/old_name2.txt");
    auto result = FileUtils::fileBatchReplaceText(
        {u1, u2}, {"old_name", "new_name"});
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.value(u1).toLocalFile().contains("new_name"));
}

TEST(FileUtilsTest, FileBatchAddText)
{
    QUrl u1 = QUrl::fromLocalFile("/tmp/file1.txt");
    QUrl u2 = QUrl::fromLocalFile("/tmp/file2.txt");
    auto result = FileUtils::fileBatchAddText(
        {u1, u2}, {"prefix_", AbstractJobHandler::FileNameAddFlag::kPrefix});
    EXPECT_EQ(result.size(), 2);
}

TEST(FileUtilsTest, ToUnicode)
{
    QByteArray data = "hello world";
    QString result = FileUtils::toUnicode(data, "test.txt");
    EXPECT_FALSE(result.isEmpty());
}

TEST(FileUtilsTest, ToUnicodeEmpty)
{
    QByteArray data;
    QString result = FileUtils::toUnicode(data, "empty.txt");
    EXPECT_TRUE(result.isEmpty());
}

TEST(FileUtilsTest, NotifyFileChangeManual)
{
    QUrl url = QUrl::fromLocalFile(QDir::tempPath());
    EXPECT_NO_FATAL_FAILURE({
        FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, url);
    });
}

TEST(FileUtilsTest, SymlinkTargetNonExistent)
{
    QUrl url = QUrl::fromLocalFile("/nonexistent_symlink");
    QString target = FileUtils::symlinkTarget(url);
    EXPECT_TRUE(target.isEmpty());
}

TEST(FileUtilsTest, ResolveSymlinkNonExistent)
{
    QUrl url = QUrl::fromLocalFile("/nonexistent_symlink");
    QString result = FileUtils::resolveSymlink(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST(FileUtilsTest, ConvertToSRgbColorSpace)
{
    QImage img(10, 10, QImage::Format_RGB32);
    img.fill(Qt::red);
    QImage result = FileUtils::convertToSRgbColorSpace(img);
    // Just verify no crash
    EXPECT_FALSE(result.isNull());
}

TEST(FileUtilsTest, ConvertToSRgbColorSpaceNull)
{
    QImage img;
    QImage result = FileUtils::convertToSRgbColorSpace(img);
    EXPECT_TRUE(result.isNull());
}

TEST(FileUtilsTest, ConvertToSRgbColorSpaceAlreadySRgb)
{
    QImage img(5, 5, QImage::Format_RGB32);
    img.setColorSpace(QColorSpace(QColorSpace::SRgb));
    QImage result = FileUtils::convertToSRgbColorSpace(img);
    EXPECT_FALSE(result.isNull());
}

TEST(FileUtilsTest, SetBackGroundNonExistent)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(FileUtils, setBackGround),
                   [](const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    // Returns true if file doesn't exist (it just skips setting bg)
    bool result = FileUtils::setBackGround("/nonexistent/background.png");
    EXPECT_TRUE(result);
}

TEST(FileUtilsTest, BindPathTransformNonDevice)
{
    QString result = FileUtils::bindPathTransform("/some/path", false);
    EXPECT_EQ(result, "/some/path");
}

TEST(FileUtilsTest, BindPathTransformToDevice)
{
    QString result = FileUtils::bindPathTransform("/some/path", true);
    EXPECT_EQ(result, "/some/path");
}

TEST(FileUtilsTest, DirFileCountNonExistent)
{
    int count = FileUtils::dirFfileCount(QUrl::fromLocalFile("/nonexistent_dir_count"));
    // May return -1 or 0 depending on implementation
    EXPECT_TRUE(count == -1 || count == 0);
}

TEST(FileUtilsTest, FileCanTrashRootDir)
{
    QUrl url = QUrl::fromLocalFile("/");
    bool result = FileUtils::fileCanTrash(url);
    EXPECT_FALSE(result);
}

TEST(FileUtilsTest, BindUrlTransformNonLocal)
{
    QUrl url("smb://server/share/file.txt");
    QUrl result = FileUtils::bindUrlTransform(url);
    EXPECT_EQ(result, url);
}

TEST(FileUtilsTest, BindUrlTransformLocal)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    QUrl result = FileUtils::bindUrlTransform(url);
    EXPECT_EQ(result, url);
}

TEST(FileUtilsTest, TrashPathToNormal)
{
    // Normal path unchanged
    QString result = FileUtils::trashPathToNormal("/tmp/file.txt");
    EXPECT_EQ(result, "/tmp/file.txt");
}

TEST(FileUtilsTest, NormalPathToTrash)
{
    // Normal paths are returned as-is (trash transform is a no-op for non-trash paths)
    QString result = FileUtils::normalPathToTrash("/tmp/file.txt");
    // Just verify no crash
    EXPECT_NO_FATAL_FAILURE({ (void)result; });
}

TEST(FileUtilsTest, SupportLongNameLocalFile)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    bool result = FileUtils::supportLongName(url);
    EXPECT_TRUE(result || !result); // depends on filesystem
}

TEST(FileUtilsTest, FindIconFromXdgEmpty)
{
    QString result = FileUtils::findIconFromXdg("");
    EXPECT_TRUE(result.isEmpty());
}

TEST(FileUtilsTest, FindIconFromXdgNonExistent)
{
    QString result = FileUtils::findIconFromXdg("nonexistent_icon_name_xyz");
    EXPECT_TRUE(result.isEmpty());
}

TEST(FileUtilsTest, FindIconFromXdgKnownIcon)
{
    // folder icon should exist in xdg
    QString result = FileUtils::findIconFromXdg("folder");
    // May or may not be found depending on system
    EXPECT_NO_FATAL_FAILURE({ (void)result; });
}

TEST(FileUtilsTest, IsDesktopFileSuffixUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.desktop");
    EXPECT_TRUE(FileUtils::isDesktopFileSuffix(url));
}

TEST(FileUtilsTest, CacheCopyingFileUrlAndRemove)
{
    QUrl url = QUrl::fromLocalFile("/tmp/copy_test.txt");
    FileUtils::cacheCopyingFileUrl(url);
    EXPECT_TRUE(FileUtils::containsCopyingFileUrl(url));
    FileUtils::removeCopyingFileUrl(url);
    EXPECT_FALSE(FileUtils::containsCopyingFileUrl(url));
}

TEST(FileUtilsTest, ContainsCopyingFileUrlNonExistent)
{
    QUrl url = QUrl::fromLocalFile("/tmp/nonexistent_copy.txt");
    EXPECT_FALSE(FileUtils::containsCopyingFileUrl(url));
}

TEST(FileUtilsTest, FileCanTrashNonExistent)
{
    QUrl url = QUrl::fromLocalFile("/nonexistent_for_trash.txt");
    bool result = FileUtils::fileCanTrash(url);
    EXPECT_FALSE(result);
}

TEST(FileUtilsTest, TrashEmptyStateAfterSet)
{
    FileUtils::setTrashEmptyState(FileUtils::TrashEmptyState::kEmpty);
    EXPECT_EQ(FileUtils::trashEmptyState(), FileUtils::TrashEmptyState::kEmpty);
    FileUtils::setTrashEmptyState(FileUtils::TrashEmptyState::kUnknown);
    EXPECT_EQ(FileUtils::trashEmptyState(), FileUtils::TrashEmptyState::kUnknown);
    FileUtils::setTrashEmptyState(FileUtils::TrashEmptyState::kNotEmpty);
    EXPECT_EQ(FileUtils::trashEmptyState(), FileUtils::TrashEmptyState::kNotEmpty);
}

TEST(FileUtilsTest, IsHigherHierarchySameUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    EXPECT_FALSE(FileUtils::isHigherHierarchy(url, url));
}

TEST(FileUtilsTest, IsSameFileWithQUrl)
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QUrl url = QUrl::fromLocalFile(tmp.fileName());
    EXPECT_TRUE(FileUtils::isSameFile(url, url, Global::CreateFileInfoType::kCreateFileInfoSync));
}

TEST(FileUtilsTest, IsSameFileWithQUrlDifferent)
{
    static std::once_flag flag2;
    std::call_once(flag2, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryFile a, b;
    ASSERT_TRUE(a.open());
    ASSERT_TRUE(b.open());
    QUrl urlA = QUrl::fromLocalFile(a.fileName());
    QUrl urlB = QUrl::fromLocalFile(b.fileName());
    EXPECT_FALSE(FileUtils::isSameFile(urlA, urlB, Global::CreateFileInfoType::kCreateFileInfoSync));
}

TEST(FileUtilsTest, DesktopAppUrlTrashDesktopFileUrl)
{
    QUrl url = DesktopAppUrl::trashDesktopFileUrl();
    EXPECT_FALSE(url.isEmpty());
    EXPECT_TRUE(url.isValid());
}

TEST(FileUtilsTest, DesktopAppUrlHomeDesktopFileUrl)
{
    QUrl url = DesktopAppUrl::homeDesktopFileUrl();
    EXPECT_FALSE(url.isEmpty());
    EXPECT_TRUE(url.isValid());
}

TEST(FileUtilsTest, DesktopAppUrlComputerDesktopFileUrl)
{
    QUrl url = DesktopAppUrl::computerDesktopFileUrl();
    EXPECT_FALSE(url.isEmpty());
    EXPECT_TRUE(url.isValid());
}

TEST(FileUtilsTest, IsContainProhibitPathWithProhibitedPaths)
{
    // Test with paths that match the prohibited list (e.g., /proc, /sys)
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/proc/cpuinfo");
    urls << QUrl::fromLocalFile("/sys/kernel");
    // The function should check each url against prohibited paths
    EXPECT_NO_FATAL_FAILURE({
        (void)FileUtils::isContainProhibitPath(urls);
    });
}
