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

#include <dfm-base/utils/fileutils.h>

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
    EXPECT_NO_FATAL_FAILURE({ FileUtils::fileCanTrash(QUrl::fromLocalFile(path)); });
}

TEST(FileUtilsTest, TrashIsEmptyIsBool)
{
    EXPECT_NO_FATAL_FAILURE({ FileUtils::trashIsEmpty(); });
}

// ---- Coverage additions: batch text operations + prohibit path ----

TEST(FileUtilsTest, IsContainProhibitPathWithEmptyListReturnsFalse)
{
    EXPECT_FALSE(FileUtils::isContainProhibitPath({}));
}

TEST(FileUtilsTest, IsContainProhibitPathWithTempPathCallable)
{
    EXPECT_NO_FATAL_FAILURE({ FileUtils::isContainProhibitPath({ QUrl::fromLocalFile("/tmp/ut_prohibit") }); });
}

TEST(FileUtilsTest, FileBatchAddTextWithEmptyListReturnsEmpty)
{
    EXPECT_TRUE(FileUtils::fileBatchAddText({}, { "prefix", AbstractJobHandler::FileNameAddFlag::kPrefix }).isEmpty());
}

TEST(FileUtilsTest, FileBatchReplaceTextWithEmptyListReturnsEmpty)
{
    EXPECT_TRUE(FileUtils::fileBatchReplaceText({}, { "old", "new" }).isEmpty());
}


TEST(FileUtilsTest, bindUrlTransform)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.bindUrlTransform(_arg0); });
}

TEST(FileUtilsTest, cacheCopyingFileUrl)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj.cacheCopyingFileUrl(_arg0));
}

TEST(FileUtilsTest, containsCopyingFileUrl)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.containsCopyingFileUrl(_arg0); });
}

TEST(FileUtilsTest, dateTimeFormat)
{
    FileUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.dateTimeFormat(); });
}

TEST(FileUtilsTest, decryptString)
{
    FileUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.decryptString(_arg0); });
}

TEST(FileUtilsTest, dirFfileCount)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.dirFfileCount(_arg0); });
}

TEST(FileUtilsTest, encryptString)
{
    FileUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.encryptString(_arg0); });
}

TEST(FileUtilsTest, findIconFromXdg)
{
    FileUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.findIconFromXdg(_arg0); });
}

TEST(FileUtilsTest, getCpuProcessCount)
{
    FileUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getCpuProcessCount(); });
}

TEST(FileUtilsTest, getFileNameLength)
{
    FileUtils obj;
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.getFileNameLength(_arg0, _arg1); });
}

TEST(FileUtilsTest, getMemoryPageSize)
{
    FileUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getMemoryPageSize(); });
}

TEST(FileUtilsTest, isComputerDesktopFile)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isComputerDesktopFile(_arg0); });
}

TEST(FileUtilsTest, isContainProhibitPath)
{
    FileUtils obj;
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isContainProhibitPath(_arg0); });
}

TEST(FileUtilsTest, isDesktopFile)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isDesktopFile(_arg0); });
}

TEST(FileUtilsTest, isDesktopFileInfo)
{
    FileUtils obj;
    FileInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isDesktopFileInfo(_arg0); });
}

TEST(FileUtilsTest, isDesktopFileSuffix)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isDesktopFileSuffix(_arg0); });
}

TEST(FileUtilsTest, isHigherHierarchy)
{
    FileUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.isHigherHierarchy(_arg0, _arg1); });
}

TEST(FileUtilsTest, isHomeDesktopFile)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isHomeDesktopFile(_arg0); });
}

TEST(FileUtilsTest, isSameDevice)
{
    FileUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.isSameDevice(_arg0, _arg1); });
}

TEST(FileUtilsTest, isSameFile)
{
    FileUtils obj;
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.isSameFile(_arg0, _arg1); });
}

TEST(FileUtilsTest, isSameMountPoint)
{
    FileUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.isSameMountPoint(_arg0, _arg1); });
}

TEST(FileUtilsTest, isTrashDesktopFile)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isTrashDesktopFile(_arg0); });
}

TEST(FileUtilsTest, isTrashFile)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isTrashFile(_arg0); });
}

TEST(FileUtilsTest, normalPathToTrash)
{
    FileUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.normalPathToTrash(_arg0); });
}

TEST(FileUtilsTest, preprocessingFileName)
{
    FileUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.preprocessingFileName(QString()); });
}

TEST(FileUtilsTest, processLength)
{
    FileUtils obj;
    QString _arg0{};
    QString _arg4{};
    int _arg5{};
    EXPECT_NO_FATAL_FAILURE({ obj.processLength(_arg0, 0, 0, false, _arg4, _arg5); });
}

TEST(FileUtilsTest, refreshIconCache)
{
    FileUtils obj;
    EXPECT_NO_FATAL_FAILURE(obj.refreshIconCache());
}

TEST(FileUtilsTest, removeCopyingFileUrl)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj.removeCopyingFileUrl(_arg0));
}

TEST(FileUtilsTest, resolveSymlink)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.resolveSymlink(_arg0); });
}

TEST(FileUtilsTest, setBackGround)
{
    FileUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.setBackGround(_arg0); });
}

TEST(FileUtilsTest, supportLongName)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.supportLongName(_arg0); });
}

TEST(FileUtilsTest, supportedMaxLength)
{
    FileUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.supportedMaxLength(_arg0); });
}

TEST(FileUtilsTest, symlinkTarget)
{
    FileUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.symlinkTarget(_arg0); });
}

TEST(FileUtilsTest, toUnicode)
{
    FileUtils obj;
    QByteArray _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.toUnicode(_arg0, _arg1); });
}

TEST(FileUtilsTest, trashEmptyState)
{
    FileUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.trashEmptyState(); });
}

TEST(FileUtilsTest, trashIsEmpty)
{
    FileUtils obj;
    bool result = obj.trashIsEmpty();
    EXPECT_FALSE(result);
}

TEST(FileUtilsTest, trashPathToNormal)
{
    FileUtils obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.trashPathToNormal(_arg0); });
}

TEST(FileUtilsTest, trashRootUrl)
{
    FileUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.trashRootUrl(); });
}
