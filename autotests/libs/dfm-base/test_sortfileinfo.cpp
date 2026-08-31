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

TEST(SortFileInfoTest, NeedsCompletionReturnsTrueWhenNotCompleted)
{
    SortFileInfo info;
    EXPECT_TRUE(info.needsCompletion());
}

TEST(SortFileInfoTest, NeedsCompletionReturnsFalseWhenCompleted)
{
    SortFileInfo info;
    info.markAsCompleted();
    EXPECT_FALSE(info.needsCompletion());
}


TEST(SortFileInfoTest, SortFileInfo)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE({ SortFileInfo obj; });
    }


TEST(SortFileInfoTest, createTime)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE({ obj.createTime(); });
}

TEST(SortFileInfoTest, customData)
{
    SortFileInfo obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.customData(_arg0); });
}

TEST(SortFileInfoTest, fileSize)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE({ obj.fileSize(); });
}

TEST(SortFileInfoTest, fileUrl)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE({ obj.fileUrl(); });
}

TEST(SortFileInfoTest, isDir)
{
    SortFileInfo obj;
    bool result = obj.isDir();
    EXPECT_FALSE(result);
}

TEST(SortFileInfoTest, isExecutable)
{
    SortFileInfo obj;
    bool result = obj.isExecutable();
    EXPECT_FALSE(result);
}

TEST(SortFileInfoTest, isFile)
{
    SortFileInfo obj;
    bool result = obj.isFile();
    EXPECT_FALSE(result);
}

TEST(SortFileInfoTest, isHide)
{
    SortFileInfo obj;
    bool result = obj.isHide();
    EXPECT_FALSE(result);
}

TEST(SortFileInfoTest, isReadable)
{
    SortFileInfo obj;
    bool result = obj.isReadable();
    EXPECT_FALSE(result);
}

TEST(SortFileInfoTest, isSymLink)
{
    SortFileInfo obj;
    bool result = obj.isSymLink();
    EXPECT_FALSE(result);
}

TEST(SortFileInfoTest, isWriteable)
{
    SortFileInfo obj;
    bool result = obj.isWriteable();
    EXPECT_FALSE(result);
}

TEST(SortFileInfoTest, lastModifiedTime)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE({ obj.lastModifiedTime(); });
}

TEST(SortFileInfoTest, searchKeyword)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE({ obj.searchKeyword(); });
}

TEST(SortFileInfoTest, setFile)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE(obj.setFile(false));
}

TEST(SortFileInfoTest, setSize)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE(obj.setSize(0));
}

TEST(SortFileInfoTest, setWriteable)
{
    SortFileInfo obj;
    EXPECT_NO_FATAL_FAILURE(obj.setWriteable(false));
}
