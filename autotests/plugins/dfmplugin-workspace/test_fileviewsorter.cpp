// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QList>

#include "utils/fileviewsorter.h"

using namespace dfmplugin_workspace;
using namespace dfmbase::Global;

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
