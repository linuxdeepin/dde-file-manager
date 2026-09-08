// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewsorter.h"
#include "models/fileitemdata.h"

#include <dfm-base/interfaces/sortfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <QUrl>
#include <QList>
#include <QHash>
#include <QFile>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileInfo>

#include <functional>

using namespace dfmplugin_workspace;
using namespace dfmbase::Global;
using namespace dfmbase;

namespace {
FileItemDataPointer makeSortItem(const QUrl &url, bool isDir, qint64 size, qint64 lastModified)
{
    auto *info = new SortFileInfo();
    info->setUrl(url);
    info->setDir(isDir);
    info->setFile(!isDir);
    info->setSize(size);
    info->setLastModifiedTime(lastModified);
    return FileItemDataPointer(new FileItemData(SortInfoPointer(info), nullptr));
}

QList<QUrl> toUrls(const char *const *names, int count)
{
    QList<QUrl> urls;
    for (int i = 0; i < count; ++i)
        urls << QUrl::fromLocalFile(QString("/ut-sorter/%1").arg(names[i]));
    return urls;
}
}   // namespace

class FileViewSorterTest : public testing::Test
{
protected:
    void SetUp() override
    {
        sorter = new FileViewSorter();
    }

    void TearDown() override
    {
        delete sorter;
    }

    FileViewSorter *sorter = nullptr;
};

// --- toItemRole (static) ---

TEST_F(FileViewSorterTest, ToItemRole_FileDisplayName_ReturnsFileName)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileDisplayNameRole), FileViewSorter::SortRole::FileName);
}

TEST_F(FileViewSorterTest, ToItemRole_FileSize_ReturnsSize)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileSizeRole), FileViewSorter::SortRole::Size);
}

TEST_F(FileViewSorterTest, ToItemRole_LastModified_ReturnsLastModified)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileLastModifiedRole), FileViewSorter::SortRole::LastModified);
}

TEST_F(FileViewSorterTest, ToItemRole_Created_ReturnsLastCreated)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileCreatedRole), FileViewSorter::SortRole::LastCreated);
}

TEST_F(FileViewSorterTest, ToItemRole_LastRead_ReturnsLastRead)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileLastReadRole), FileViewSorter::SortRole::LastRead);
}

TEST_F(FileViewSorterTest, ToItemRole_MimeType_ReturnsMimeType)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileMimeTypeRole), FileViewSorter::SortRole::MimeType);
}

TEST_F(FileViewSorterTest, ToItemRole_FilePath_ReturnsFilePath)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFilePathRole), FileViewSorter::SortRole::FilePath);
}

TEST_F(FileViewSorterTest, ToItemRole_OriginalPath_ReturnsOriginalPath)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileOriginalPath), FileViewSorter::SortRole::OriginalPath);
}

TEST_F(FileViewSorterTest, ToItemRole_DeletionDate_ReturnsDeletionDate)
{
    EXPECT_EQ(FileViewSorter::toItemRole(kItemFileDeletionDate), FileViewSorter::SortRole::DeletionDate);
}

TEST_F(FileViewSorterTest, ToItemRole_UnknownRole_ReturnsFileName)
{
    EXPECT_EQ(FileViewSorter::toItemRole(static_cast<ItemRoles>(99999)), FileViewSorter::SortRole::FileName);
}

// --- setContext ---

TEST_F(FileViewSorterTest, SetContext_Defaults)
{
    FileViewSorter::SortContext ctx;
    sorter->setContext(ctx);
    // Just verify no crash; context is private
    SUCCEED();
}

TEST_F(FileViewSorterTest, SetContext_WithCustomValues)
{
    FileViewSorter::SortContext ctx;
    ctx.rootUrl = QUrl("file:///home");
    ctx.isMixDirAndFile = true;
    ctx.order = Qt::DescendingOrder;
    ctx.role = FileViewSorter::SortRole::Size;
    ctx.isUnderHomeDir = true;
    ctx.checkDesktopFile = true;
    sorter->setContext(ctx);
    SUCCEED();
}

// --- reverse (uses m_context) ---

TEST_F(FileViewSorterTest, Reverse_EmptyList_ReturnsEmpty)
{
    QList<QUrl> empty;
    EXPECT_EQ(sorter->reverse(empty).size(), 0);
}

TEST_F(FileViewSorterTest, Reverse_MixedList_NoCrash)
{
    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::FileName;
    sorter->setContext(ctx);

    QList<QUrl> urls = {QUrl("file:///a"), QUrl("file:///b"), QUrl("file:///c")};
    EXPECT_NO_FATAL_FAILURE(sorter->reverse(urls));
}

TEST_F(FileViewSorterTest, Reverse_SingleElement_ReturnsSingle)
{
    QList<QUrl> urls = {QUrl("file:///a")};
    QList<QUrl> result = sorter->reverse(urls);
    EXPECT_EQ(result.size(), 1);
}

TEST_F(FileViewSorterTest, Reverse_SeparatedMode_NoCrash)
{
    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = false;
    ctx.role = FileViewSorter::SortRole::FileName;
    sorter->setContext(ctx);

    QList<QUrl> urls = {QUrl("file:///a"), QUrl("file:///b")};
    EXPECT_NO_FATAL_FAILURE(sorter->reverse(urls));
}

TEST_F(FileViewSorterTest, Reverse_SizeRole_NoCrash)
{
    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::Size;
    sorter->setContext(ctx);

    QList<QUrl> urls = {QUrl("file:///a"), QUrl("file:///b"), QUrl("file:///c")};
    EXPECT_NO_FATAL_FAILURE(sorter->reverse(urls));
}

// --- sort (mixed / separated) ---

TEST_F(FileViewSorterTest, Sort_MixedFileNameAscending_OrdersByName)
{
    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::FileName;
    ctx.order = Qt::AscendingOrder;
    sorter->setContext(ctx);

    const char *names[] = { "c.txt", "a.txt", "b.txt" };
    QList<QUrl> result = sorter->sort(toUrls(names, 3));

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result.at(0).fileName(), QString("a.txt"));
    EXPECT_EQ(result.at(2).fileName(), QString("c.txt"));
}

TEST_F(FileViewSorterTest, Sort_MixedFileNameDescending_ReversesOrder)
{
    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::FileName;
    ctx.order = Qt::DescendingOrder;
    sorter->setContext(ctx);

    const char *names[] = { "c.txt", "a.txt", "b.txt" };
    QList<QUrl> result = sorter->sort(toUrls(names, 3));

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result.at(0).fileName(), QString("c.txt"));
    EXPECT_EQ(result.at(2).fileName(), QString("a.txt"));
}

TEST_F(FileViewSorterTest, Sort_SeparatedMode_DirectoriesBeforeFiles)
{
    QHash<QString, FileItemDataPointer> items;
    auto add = [&items](const QString &name, bool dir) {
        QUrl url = QUrl::fromLocalFile("/ut-sorter/" + name);
        items.insert(name, makeSortItem(url, dir, 10, 1000));
        return url;
    };
    QUrl dirZ = add("zdir", true);
    QUrl dirA = add("adir", true);
    QUrl fileB = add("bfile", false);
    QUrl fileA = add("afile", false);

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = false;
    ctx.role = FileViewSorter::SortRole::FileName;
    ctx.order = Qt::AscendingOrder;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.path()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ dirZ, dirA, fileB, fileA });

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result.at(0), dirA);
    EXPECT_EQ(result.at(1), dirZ);
    EXPECT_EQ(result.at(2), fileA);
    EXPECT_EQ(result.at(3), fileB);
}

TEST_F(FileViewSorterTest, Sort_SizeRoleAscending_DirectoryFirstThenBySize)
{
    QUrl dirUrl = QUrl::fromLocalFile("/ut-sorter/dirA");
    QUrl f100 = QUrl::fromLocalFile("/ut-sorter/f100.bin");
    QUrl f500 = QUrl::fromLocalFile("/ut-sorter/f500.bin");
    QUrl f050 = QUrl::fromLocalFile("/ut-sorter/f050.bin");
    QHash<QString, FileItemDataPointer> items;
    items.insert("dirA", makeSortItem(dirUrl, true, 0, 0));
    items.insert("f100.bin", makeSortItem(f100, false, 100, 0));
    items.insert("f500.bin", makeSortItem(f500, false, 500, 0));
    items.insert("f050.bin", makeSortItem(f050, false, 50, 0));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::Size;
    ctx.order = Qt::AscendingOrder;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.path()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ f500, dirUrl, f100, f050 });

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result.at(0), dirUrl);   // directory always prefixed first on ascending
    EXPECT_EQ(result.at(1), f050);
    EXPECT_EQ(result.at(3), f500);
}

TEST_F(FileViewSorterTest, Sort_LastModifiedRole_OrdersByTimeWithFallback)
{
    QUrl early = QUrl::fromLocalFile("/ut-sorter/early.txt");
    QUrl late = QUrl::fromLocalFile("/ut-sorter/late.txt");
    QUrl noTime = QUrl::fromLocalFile("/ut-sorter/notime.txt");
    QHash<QString, FileItemDataPointer> items;
    items.insert("early.txt", makeSortItem(early, false, 1, 1000));
    items.insert("late.txt", makeSortItem(late, false, 1, 2000000000));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::LastModified;
    ctx.order = Qt::AscendingOrder;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.path()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ late, noTime, early });

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result.at(0), noTime);   // fallback default time "0000/00/00" sorts first
    EXPECT_EQ(result.at(1), early);
    EXPECT_EQ(result.at(2), late);
}

TEST_F(FileViewSorterTest, Sort_TimeRolesWithoutCallback_UseDefaultTime)
{
    const FileViewSorter::SortRole roles[] = {
        FileViewSorter::SortRole::LastCreated,
        FileViewSorter::SortRole::LastRead,
        FileViewSorter::SortRole::DeletionDate
    };
    for (auto role : roles) {
        FileViewSorter::SortContext ctx;
        ctx.isMixDirAndFile = true;
        ctx.role = role;
        sorter->setContext(ctx);

        const char *names[] = { "x.txt", "a.txt" };
        QList<QUrl> result = sorter->sort(toUrls(names, 2));
        ASSERT_EQ(result.size(), 2) << "role=" << int(role);
        // all keys share the same fallback time, secondary key is the file name
        EXPECT_EQ(result.at(0).fileName(), QString("a.txt")) << "role=" << int(role);
    }
}

TEST_F(FileViewSorterTest, Sort_FilePathRole_UsesLocalPathAsKey)
{
    QUrl urlCa = QUrl::fromLocalFile("/ut-sorter/c/a");
    QUrl urlAb = QUrl::fromLocalFile("/ut-sorter/a/b");
    QUrl urlBc = QUrl::fromLocalFile("/ut-sorter/b/c");
    QHash<QString, FileItemDataPointer> items;
    items.insert("a", makeSortItem(urlAb, false, 1, 0));
    items.insert("b", makeSortItem(urlBc, false, 1, 0));
    items.insert("c", makeSortItem(urlCa, false, 1, 0));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::FilePath;
    ctx.order = Qt::AscendingOrder;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QString(QFileInfo(url.path()).fileName()));
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ urlCa, urlAb, urlBc });

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result.at(0).path(), QString("/ut-sorter/a/b"));
    EXPECT_EQ(result.at(2).path(), QString("/ut-sorter/c/a"));
}

TEST_F(FileViewSorterTest, Sort_OriginalPathRoleWithoutInfo_FallsBackToUrlPath)
{
    QUrl urlY = QUrl::fromLocalFile("/ut-sorter/y");
    QUrl urlZ = QUrl::fromLocalFile("/ut-sorter/z");
    QHash<QString, FileItemDataPointer> items;
    items.insert("y", makeSortItem(urlY, false, 1, 0));
    items.insert("z", makeSortItem(urlZ, false, 1, 0));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::OriginalPath;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QString(QFileInfo(url.path()).fileName()));
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ urlZ, urlY });

    ASSERT_EQ(result.size(), 2);
    // no trash originalUrl data: both keys are empty, input order is preserved
    EXPECT_EQ(result.at(0), urlZ);
    EXPECT_EQ(result.at(1), urlY);
}

TEST_F(FileViewSorterTest, Sort_MimeTypeRoleNoInfo_RanksUnknownGroupBytName)
{
    QUrl urlB = QUrl::fromLocalFile("/ut-sorter/b.doc");
    QUrl urlA = QUrl::fromLocalFile("/ut-sorter/a.doc");
    QUrl urlC = QUrl::fromLocalFile("/ut-sorter/c.doc");
    QHash<QString, FileItemDataPointer> items;
    items.insert("b.doc", makeSortItem(urlB, false, 1, 0));
    items.insert("a.doc", makeSortItem(urlA, false, 1, 0));
    items.insert("c.doc", makeSortItem(urlC, false, 1, 0));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::MimeType;
    ctx.order = Qt::AscendingOrder;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.path()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ urlB, urlC, urlA });

    ASSERT_EQ(result.size(), 3);
    // no FileInfo and no SortInfo dates: all unknown mime, tie broken by name
    EXPECT_EQ(result.at(0), urlA);
    EXPECT_EQ(result.at(2), urlC);
}

TEST_F(FileViewSorterTest, Sort_MimeTypeRoleRealFiles_UsesAccurateMimeName)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString txtPath = dir.filePath("note.txt");
    QString pngPath = dir.filePath("pic.png");
    QFile txtFile(txtPath);
    QFile pngFile(pngPath);
    ASSERT_TRUE(txtFile.open(QIODevice::WriteOnly));
    ASSERT_TRUE(pngFile.open(QIODevice::WriteOnly));
    txtFile.close();
    pngFile.close();

    QUrl txtUrl = QUrl::fromLocalFile(txtPath);
    QUrl pngUrl = QUrl::fromLocalFile(pngPath);
    QHash<QString, FileItemDataPointer> items;
    items.insert("note.txt", makeSortItem(txtUrl, false, 1, 1000));
    items.insert("pic.png", makeSortItem(pngUrl, false, 1, 1000));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::MimeType;
    ctx.order = Qt::AscendingOrder;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.fileName()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ pngUrl, txtUrl });

    ASSERT_EQ(result.size(), 2);
    // image group rank (2) sorts after text group rank (1)
    EXPECT_EQ(result.at(0), txtUrl);
    EXPECT_EQ(result.at(1), pngUrl);
}

TEST_F(FileViewSorterTest, Sort_FileNameUnderHomeWithoutInfo_UsesUrlFileName)
{
    QUrl urlB = QUrl::fromLocalFile("/ut-sorter/home/b");
    QUrl urlA = QUrl::fromLocalFile("/ut-sorter/home/a");
    QHash<QString, FileItemDataPointer> items;
    items.insert("b", makeSortItem(urlB, false, 1, 0));
    items.insert("a", makeSortItem(urlA, false, 1, 0));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::FileName;
    ctx.isUnderHomeDir = true;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.path()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ urlB, urlA });

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0), urlA);
    EXPECT_EQ(result.at(1), urlB);
}

TEST_F(FileViewSorterTest, Sort_LastModifiedFromFileInfo_UsesFileStatTime)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString pathA = dir.filePath("older.txt");
    QFile fileA(pathA);
    ASSERT_TRUE(fileA.open(QIODevice::WriteOnly));
    fileA.write("hello");
    fileA.close();
    QString pathB = dir.filePath("newer.txt");
    QFile fileB(pathB);
    ASSERT_TRUE(fileB.open(QIODevice::WriteOnly));
    fileB.write("world!!");
    fileB.close();

    QUrl urlA = QUrl::fromLocalFile(pathA);
    QUrl urlB = QUrl::fromLocalFile(pathB);
    QHash<QString, FileItemDataPointer> items;
    // no SortInfo: the sorter must fall back to FileInfo stat times
    items.insert("older.txt", FileItemDataPointer(new FileItemData(urlA, InfoFactory::create<FileInfo>(urlA))));
    items.insert("newer.txt", FileItemDataPointer(new FileItemData(urlB, InfoFactory::create<FileInfo>(urlB))));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::LastModified;
    ctx.order = Qt::AscendingOrder;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.fileName()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->sort({ urlB, urlA });

    ASSERT_EQ(result.size(), 2);
    // both created moments ago; the key is deterministic "time_filename"
    EXPECT_TRUE(result.contains(urlA));
    EXPECT_TRUE(result.contains(urlB));
}

// --- findInsertPosition ---

TEST_F(FileViewSorterTest, FindInsertPosition_MiddleOfSortedList_ReturnsOne)
{
    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::FileName;
    ctx.order = Qt::AscendingOrder;
    sorter->setContext(ctx);

    QList<QUrl> sorted = toUrls(nullptr, 0);
    sorted << QUrl::fromLocalFile("/ut-sorter/a.txt")
           << QUrl::fromLocalFile("/ut-sorter/c.txt");
    int pos = sorter->findInsertPosition(QUrl::fromLocalFile("/ut-sorter/b.txt"), sorted);

    EXPECT_EQ(pos, 1);
    EXPECT_EQ(sorter->findInsertPosition(QUrl::fromLocalFile("/d.txt"), {}), 0);
}

TEST_F(FileViewSorterTest, FindInsertPosition_DescendingOrder_FindsTail)
{
    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = true;
    ctx.role = FileViewSorter::SortRole::FileName;
    ctx.order = Qt::DescendingOrder;
    sorter->setContext(ctx);

    QList<QUrl> sorted;
    sorted << QUrl::fromLocalFile("/ut-sorter/c.txt")
           << QUrl::fromLocalFile("/ut-sorter/a.txt");
    int pos = sorter->findInsertPosition(QUrl::fromLocalFile("/ut-sorter/b.txt"), sorted);

    EXPECT_EQ(pos, 1);
    EXPECT_EQ(sorted.size(), 2);
}

// --- reverse grouped ---

TEST_F(FileViewSorterTest, Reverse_SeparatedMode_ReversesInsideGroups)
{
    QUrl dirA = QUrl::fromLocalFile("/ut-sorter/adir");
    QUrl dirB = QUrl::fromLocalFile("/ut-sorter/bdir");
    QUrl fileA = QUrl::fromLocalFile("/ut-sorter/afile");
    QUrl fileB = QUrl::fromLocalFile("/ut-sorter/bfile");
    QHash<QString, FileItemDataPointer> items;
    items.insert("adir", makeSortItem(dirA, true, 0, 0));
    items.insert("bdir", makeSortItem(dirB, true, 0, 0));
    items.insert("afile", makeSortItem(fileA, false, 0, 0));
    items.insert("bfile", makeSortItem(fileB, false, 0, 0));

    FileViewSorter::SortContext ctx;
    ctx.isMixDirAndFile = false;
    ctx.role = FileViewSorter::SortRole::FileName;
    ctx.getDataCallback = [&items](const QUrl &url) {
        return items.value(QFileInfo(url.path()).fileName());
    };
    sorter->setContext(ctx);

    QList<QUrl> result = sorter->reverse({ dirA, dirB, fileA, fileB });

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result.at(0), dirB);
    EXPECT_EQ(result.at(1), dirA);
    EXPECT_EQ(result.at(2), fileB);
    EXPECT_EQ(result.at(3), fileA);
}
