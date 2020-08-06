#include "models/recentfileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestRecentFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestRecentFileInfo";
        info = new RecentFileInfo(DUrl("recent:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestRecentFileInfo";
    }

public:
    RecentFileInfo *info;
};
} // namespace

TEST_F(TestRecentFileInfo, makeAbsolute)
{
    EXPECT_TRUE(info->makeAbsolute());
}

TEST_F(TestRecentFileInfo, fileExist)
{
    EXPECT_TRUE(info->exists());
}

TEST_F(TestRecentFileInfo, fileIsDir)
{
    EXPECT_FALSE(info->isDir());
}

TEST_F(TestRecentFileInfo, fileIsReadable)
{
    EXPECT_TRUE(info->isReadable());
}

TEST_F(TestRecentFileInfo, fileIsWritable)
{
    EXPECT_FALSE(info->isWritable());
}

TEST_F(TestRecentFileInfo, fileCanIterator)
{
    EXPECT_TRUE(info->canIteratorDir());
}

TEST_F(TestRecentFileInfo, fileCanDrop)
{
    EXPECT_FALSE(info->canDrop());
}

TEST_F(TestRecentFileInfo, fileCanRedirectUrl)
{
    EXPECT_TRUE(info->canRedirectionFileUrl());
}

TEST_F(TestRecentFileInfo, fileCanRename)
{
    EXPECT_FALSE(info->canRename());
}

TEST_F(TestRecentFileInfo, redirectedUrl)
{
    EXPECT_STREQ("/test.file", info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, getUrlByNewFileName)
{
    EXPECT_STREQ("/", info->getUrlByNewFileName("").path().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, toLocalFile)
{
    EXPECT_STREQ("/test.file", info->toLocalFile().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, subtitleForEmptyFolder)
{
    EXPECT_STREQ("Folder is empty", info->subtitleForEmptyFloder().toStdString().c_str());
}

//TEST_F(TestRecentFileInfo, gotoUrlWhenDeleted) {
//    EXPECT_STREQ("/home/xust", info->goToUrlWhenDeleted().path().toStdString().c_str());
//}

TEST_F(TestRecentFileInfo, fileDisplayName)
{
    EXPECT_STREQ("", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, mimeTypeDisplayName)
{
    EXPECT_STREQ("Unknown ()", info->mimeTypeDisplayName().toStdString().c_str());
}

TEST_F(TestRecentFileInfo, isVirtualEntry)
{
    EXPECT_FALSE(info->isVirtualEntry());
}

TEST_F(TestRecentFileInfo, increaseCovrage)
{
    QDateTime curr = QDateTime::currentDateTime();
    info->updateReadTime(curr);
    EXPECT_STREQ(curr.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str(),
                 info->getReadTime().toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
}
