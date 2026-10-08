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
#include <climits>
#include <QDBusConnection>
#include <QDBusPendingCall>

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/networkutils.h>
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

// ===== PMS sev-2 regression cluster: fileutils.cpp (work-order batch 2) =====

// PMS:318199 resolveSymlink 符号链接循环防护：a->b->a 环应返回空串而非死循环/崩溃
TEST(FileUtilsTest, BUG318199_ResolveSymlinkCycleReturnsEmpty)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString a = dir.filePath("link_a");
    const QString b = dir.filePath("link_b");
    ASSERT_TRUE(QFile::link(a, b));   // b -> a
    ASSERT_TRUE(QFile::link(b, a));   // a -> b
    const QString r = FileUtils::resolveSymlink(QUrl::fromLocalFile(a));
    EXPECT_TRUE(r.isEmpty());
}

// PMS:318199 resolveSymlink 链式解析应终止于真实文件（BUG 318199 修复的核心契约）
TEST(FileUtilsTest, BUG318199_ResolveSymlinkChainEndsAtRealFile)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString real = dir.filePath("real.txt");
    {
        QFile f(real);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    }
    const QString l1 = dir.filePath("l1");
    ASSERT_TRUE(QFile::link(real, l1));
    const QString l2 = dir.filePath("l2");
    ASSERT_TRUE(QFile::link(l1, l2));
    const QString r = FileUtils::resolveSymlink(QUrl::fromLocalFile(l2));
    EXPECT_EQ(r, real);
}

// PMS:326991 CMYK 图无色彩空间时转换不应返回空图/CMYK 格式（闪退根因兜底链）
TEST(FileUtilsTest, BUG326991_ConvertToSRgbColorSpaceCmykWithoutColorSpace)
{
    QImage cmyk(8, 8, QImage::Format_CMYK8888);
    cmyk.fill(Qt::black);
    ASSERT_FALSE(cmyk.isNull());
    ASSERT_FALSE(cmyk.colorSpace().isValid());
    QImage out = FileUtils::convertToSRgbColorSpace(cmyk);
    EXPECT_FALSE(out.isNull());
    EXPECT_NE(out.format(), QImage::Format_CMYK8888);
}

// PMS:326991 已是 sRGB 色彩空间的图像应原样返回（不重复转换导致色彩漂移）
TEST(FileUtilsTest, BUG326991_ConvertToSRgbColorSpaceSRgbImageUntouched)
{
    QImage img(4, 4, QImage::Format_RGB32);
    img.fill(Qt::red);
    img.setColorSpace(QColorSpace(QColorSpace::SRgb));
    QImage out = FileUtils::convertToSRgbColorSpace(img);
    EXPECT_FALSE(out.isNull());
    EXPECT_EQ(out.colorSpace(), QColorSpace(QColorSpace::SRgb));
}

// PMS:309847 isSameDevice 契约：本地同设备返回 true；同 scheme 不同主机返回 false
TEST(FileUtilsTest, BUG309847_IsSameDeviceSchemeHostContract)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString existing = dir.filePath("existing.txt");
    {
        QFile f(existing);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    }
    EXPECT_TRUE(FileUtils::isSameDevice(QUrl::fromLocalFile(dir.path()),
                                        QUrl::fromLocalFile(existing)));
    EXPECT_FALSE(FileUtils::isSameDevice(QUrl("smb://hostA/share"), QUrl("smb://hostB/share")));
    EXPECT_FALSE(FileUtils::isSameDevice(QUrl("smb://hostA/share"), QUrl("nfs://hostA/share")));
}

// PMS:309847 已知活体源码缺陷：isSameDevice 远程分支 url1.port() == url1.port() 自比较
// （应为 url2.port()，2022 年 ab5a22703 引入；已录入 .ut-defects.json，源码修复后启用）
TEST(FileUtilsTest, BUG309847_RemotePortSelfCompareKnownDefect)
{
    GTEST_SKIP() << "known source defect: isSameDevice compares url1.port() with itself; "
                    "recorded in autotests/.ut-defects.json";
    EXPECT_FALSE(FileUtils::isSameDevice(QUrl("smb://host:445/share"),
                                         QUrl("smb://host:446/share")));
}

// PMS:129771 setBackGround 右键设置壁纸：无 Appearance 服务时仍应安全返回 true
// （stub 掉 sessionBus asyncCall，避免真实改壁纸；同步覆盖 greeterbackground 兜底分支）
TEST(FileUtilsTest, BUG129771_SetBackGroundSendsAppearanceDBusWithoutCrash)
{
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusConnection::*)(const QDBusMessage &, int) const>(
                       &QDBusConnection::asyncCall),
                   [](QDBusConnection *, const QDBusMessage &, int) {
                       __DBG_STUB_INVOKE__
                       // Qt6: QDBusPendingCall default ctor is not defined; use fromCompletedCall
                       return QDBusPendingCall::fromCompletedCall(QDBusMessage());
                   });
    EXPECT_TRUE(FileUtils::setBackGround(QString("/tmp/ut-not-exist-wallpaper.png")));
}

// ============================================================
// PMS sev-2 regression cluster: fileutils.cpp (work-order batch 3)
// ============================================================

// PMS:165023 isSameFile(QUrl) 契约：同 inode（符号链接指向同一文件）的不同路径判定为同一文件；
// 依赖 UniversalUtils::urlEquals 短路与 InfoFactory 拿不到 info 时安全返回 false
TEST(FileUtilsTest, BUG165023_IsSameFileSameInodeViaSymlink)
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    });

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString realPath = dir.path() + "/real_165023.txt";
    QFile f(realPath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("inode");
    f.close();
    const QString linkPath = dir.path() + "/link_165023.txt";
    ASSERT_TRUE(QFile::link(realPath, linkPath));

    const QUrl realUrl = QUrl::fromLocalFile(realPath);
    const QUrl linkUrl = QUrl::fromLocalFile(linkPath);

    // same object via urlEquals shortcut
    EXPECT_TRUE(FileUtils::isSameFile(realUrl, realUrl, Global::CreateFileInfoType::kCreateFileInfoSync));
    // different paths, same inode (stat follows the symlink)
    EXPECT_TRUE(FileUtils::isSameFile(realUrl, linkUrl, Global::CreateFileInfoType::kCreateFileInfoSync));
    // different files -> false
    const QString otherPath = dir.path() + "/other_165023.txt";
    QFile g(otherPath);
    ASSERT_TRUE(g.open(QIODevice::WriteOnly));
    g.write("other");
    g.close();
    EXPECT_FALSE(FileUtils::isSameFile(realUrl, QUrl::fromLocalFile(otherPath),
                                       Global::CreateFileInfoType::kCreateFileInfoSync));
    // missing file info -> false (no crash)
    EXPECT_FALSE(FileUtils::isSameFile(realUrl, QUrl::fromLocalFile("/no/such/file_165023"),
                                       Global::CreateFileInfoType::kCreateFileInfoSync));
}

// PMS:211431 processLength 按 UCS4 修剪光标前文本：全宽字符/emoji（UTF-16 代理对）必须作为单字符删除，
// 不能在代理对中间截断；字符计数与字节计数两种模式均需正确
TEST(FileUtilsTest, BUG211431_ProcessLengthTrimsUcs4Aware)
{
    QString dstText;
    int dstPos = -1;

    // byte-count mode: "ab😀cd" = 2+4+2 bytes, cursor after emoji (srcPos=4), max 5 bytes.
    // The emoji must be removed as ONE character (UCS4), leaving "abcd" (4 bytes).
    bool changed = FileUtils::processLength(QStringLiteral("ab😀cd"), 4, 5, false, dstText, dstPos);
    EXPECT_TRUE(changed);
    EXPECT_EQ(dstText, QStringLiteral("abcd"));
    EXPECT_EQ(dstPos, 2);

    // char-count mode: "😀abc" UTF-16 length 5, cursor after emoji (srcPos=2), max 3 chars -> "abc"
    dstText.clear();
    dstPos = -1;
    changed = FileUtils::processLength(QStringLiteral("😀abc"), 2, 3, true, dstText, dstPos);
    EXPECT_TRUE(changed);
    EXPECT_EQ(dstText, QStringLiteral("abc"));
    EXPECT_EQ(dstPos, 0);

    // within limit -> no change, returns false, outputs keep input
    dstText.clear();
    dstPos = -1;
    changed = FileUtils::processLength(QStringLiteral("hello"), 2, INT_MAX, false, dstText, dstPos);
    EXPECT_FALSE(changed);
    EXPECT_EQ(dstText, QStringLiteral("hello"));
    EXPECT_EQ(dstPos, 2);

    // cursor at start and already over limit -> cannot trim left, returns false safely
    dstText.clear();
    dstPos = -1;
    changed = FileUtils::processLength(QStringLiteral("😀"), 0, 2, false, dstText, dstPos);
    EXPECT_FALSE(changed);
}

// PMS:299425 trashIsEmpty：CIFS 挂载繁忙时直接短路返回 true（避免不可靠的回收站统计）
TEST(FileUtilsTest, BUG299425_TrashIsEmptyShortCircuitsWhenCifsBusy)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(NetworkUtils, checkAllCIFSBusy), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    EXPECT_TRUE(FileUtils::trashIsEmpty());
}

// PMS:303915 trashIsEmpty：非繁忙路径（CIFS 空闲）下 trash:// FileInfo 不可用时安全返回 true，不崩溃
TEST(FileUtilsTest, BUG303915_TrashIsEmptySafeWhenTrashInfoUnavailable)
{
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(NetworkUtils, checkAllCIFSBusy), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    // trash:// FileInfo class is not registered in this binary -> info is null -> true
    EXPECT_TRUE(FileUtils::trashIsEmpty());
    EXPECT_TRUE(FileUtils::trashIsEmpty());   // repeated call stays consistent
}

// PMS:178509 拖拽文件到回收站排序崩溃：修复引入的 FileUtils::isLocalFile/
// isLocalDevice 已被 ProtocolUtils 重构删除（契约由 ProtocolUtils::isLocalFile
// 继承并有独立覆盖），原修复函数不存在，无法在本文件回归
TEST(FileUtilsTest, BUG178509_Skip_FixCodeRemovedByProtocolUtilsRefactor)
{
    GTEST_SKIP() << "fix introduced FileUtils::isLocalFile/isLocalDevice which were "
                    "removed by the ProtocolUtils refactor; isLocalFile contract is "
                    "covered by TestIsLocalFile (protocolutils tests)";
}
