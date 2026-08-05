// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortfileinfo.cpp
 * @brief Unit tests for SortFileInfo (sortfileinfo.cpp)
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariant>

#include <dfm-base/interfaces/sortfileinfo.h>

using namespace dfmbase;

TEST(SortFileInfoTest, SetAndGetUrl)
{
    SortFileInfo info;
    QUrl url("file:///home/user/test.txt");
    info.setUrl(url);
    EXPECT_EQ(info.fileUrl(), url);
}

TEST(SortFileInfoTest, SetAndGetSize)
{
    SortFileInfo info;
    info.setSize(1024);
    EXPECT_EQ(info.fileSize(), 1024);
}

TEST(SortFileInfoTest, SetIsFile)
{
    SortFileInfo info;
    info.setFile(true);
    EXPECT_TRUE(info.isFile());
    EXPECT_FALSE(info.isDir());
}

TEST(SortFileInfoTest, SetIsDir)
{
    SortFileInfo info;
    info.setDir(true);
    EXPECT_TRUE(info.isDir());
    EXPECT_FALSE(info.isFile());
}

TEST(SortFileInfoTest, SetIsSymlink)
{
    SortFileInfo info;
    info.setSymlink(true);
    EXPECT_TRUE(info.isSymLink());
}

TEST(SortFileInfoTest, SetIsHide)
{
    SortFileInfo info;
    info.setHide(true);
    EXPECT_TRUE(info.isHide());
}

TEST(SortFileInfoTest, SetReadableWriteableExecutable)
{
    SortFileInfo info;
    info.setReadable(true);
    info.setWriteable(true);
    info.setExecutable(true);
    EXPECT_TRUE(info.isReadable());
    EXPECT_TRUE(info.isWriteable());
    EXPECT_TRUE(info.isExecutable());
}

TEST(SortFileInfoTest, SetTimes)
{
    SortFileInfo info;
    info.setLastReadTime(1000);
    info.setLastModifiedTime(2000);
    info.setCreateTime(3000);
    EXPECT_EQ(info.lastReadTime(), 1000);
    EXPECT_EQ(info.lastModifiedTime(), 2000);
    EXPECT_EQ(info.createTime(), 3000);
}

TEST(SortFileInfoTest, SetHighlightContent)
{
    SortFileInfo info;
    info.setHighlightContent("highlight me");
    EXPECT_EQ(info.highlightContent(), QString("highlight me"));
}

TEST(SortFileInfoTest, SetSearchKeywordAndType)
{
    SortFileInfo info;
    info.setSearchKeyword("kw");
    info.setSearchType(2);
    EXPECT_EQ(info.searchKeyword(), QString("kw"));
    EXPECT_EQ(info.searchType(), 2);
}

TEST(SortFileInfoTest, SetCustomData)
{
    SortFileInfo info;
    info.setCustomData("key1", QString("value1"));
    EXPECT_EQ(info.customData("key1").toString(), QString("value1"));
}

TEST(SortFileInfoTest, SetInfoCompleted)
{
    SortFileInfo info;
    EXPECT_FALSE(info.isInfoCompleted());
    info.setInfoCompleted(true);
    EXPECT_TRUE(info.isInfoCompleted());
}

TEST(SortFileInfoTest, MarkAsCompleted)
{
    SortFileInfo info;
    info.markAsCompleted();
    EXPECT_TRUE(info.isInfoCompleted());
}
