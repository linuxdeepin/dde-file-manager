#include "models/searchfileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestSearchFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestSearchFileInfo";
        info = new SearchFileInfo(DUrl("search:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestSearchFileInfo";
    }

public:
    SearchFileInfo *info;
};
} // namespace

TEST_F(TestSearchFileInfo, fileExist)
{
    EXPECT_TRUE(info->exists());
}

TEST_F(TestSearchFileInfo, fileIsReadable)
{
    EXPECT_TRUE(info->isReadable());
}

TEST_F(TestSearchFileInfo, fileIsDir)
{
    EXPECT_TRUE(info->isDir());
}

TEST_F(TestSearchFileInfo, fileIsVirtualEntry)
{
    EXPECT_TRUE(info->isVirtualEntry());
}

TEST_F(TestSearchFileInfo, filesCount)
{
    EXPECT_EQ(0, info->filesCount());
}

TEST_F(TestSearchFileInfo, parentUrl)
{
    EXPECT_STREQ("", info->parentUrl().path().toStdString().c_str());
}

TEST_F(TestSearchFileInfo, canRedirectUrl)
{
    EXPECT_FALSE(info->canRedirectionFileUrl());
}

TEST_F(TestSearchFileInfo, redirectedUrl)
{
    EXPECT_STREQ("", info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestSearchFileInfo, fileCanIterator)
{
    EXPECT_TRUE(info->canIteratorDir());
}

TEST_F(TestSearchFileInfo, isEmptyFolder)
{
    EXPECT_FALSE(info->isEmptyFloder());
}

TEST_F(TestSearchFileInfo, hasOrder)
{
    EXPECT_FALSE(info->hasOrderly());
}

TEST_F(TestSearchFileInfo, loadingTip)
{
    EXPECT_STREQ("Searching...", info->loadingTip().toStdString().c_str());
}

TEST_F(TestSearchFileInfo, subtitleForEmptyFolder)
{
    EXPECT_STREQ("No results", info->subtitleForEmptyFloder().toStdString().c_str());
}

TEST_F(TestSearchFileInfo, fileDisplayName)
{
    EXPECT_STREQ("Search", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestSearchFileInfo, mimeDataUrl)
{
    EXPECT_STREQ("", info->mimeDataUrl().path().toStdString().c_str());
}

TEST_F(TestSearchFileInfo, toLocalFile)
{
    EXPECT_STREQ("", info->toLocalFile().toStdString().c_str());
}
