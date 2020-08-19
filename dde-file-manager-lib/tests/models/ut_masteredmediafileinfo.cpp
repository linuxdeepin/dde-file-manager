#include "models/masteredmediafileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestMasteredMediaFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMasteredMediaFileInfo";

        info = new MasteredMediaFileInfo(DUrl("burn:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestMasteredMediaFileInfo";
    }

public:
    MasteredMediaFileInfo *info;
};
} // namespace

TEST_F(TestMasteredMediaFileInfo, fileDoNotExist)
{
    EXPECT_FALSE(info->exists());
}

TEST_F(TestMasteredMediaFileInfo, fileIsReadable)
{
    EXPECT_TRUE(info->isReadable());
}

TEST_F(TestMasteredMediaFileInfo, fileIsWritable)
{
    EXPECT_TRUE(info->isWritable());
}

TEST_F(TestMasteredMediaFileInfo, fileIsDir)
{
    EXPECT_TRUE(info->isDir());
}

TEST_F(TestMasteredMediaFileInfo, filesCount)
{
    EXPECT_EQ(0, info->filesCount());
}

//TEST_F(TestMasteredMediaFileInfo, fileDisplayName) {
//    EXPECT_STREQ("test", info->fileDisplayName().toStdString().c_str());
//}

TEST_F(TestMasteredMediaFileInfo, fileCanRedirectUrl)
{
    EXPECT_FALSE(info->canRedirectionFileUrl());
}

TEST_F(TestMasteredMediaFileInfo, redirectUrl)
{
    EXPECT_STREQ("/test.file", info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestMasteredMediaFileInfo, mimeDataUrl)
{
    EXPECT_STREQ("/test.file", info->mimeDataUrl().path().toStdString().c_str());
}

TEST_F(TestMasteredMediaFileInfo, canIteratorDir)
{
    EXPECT_TRUE(info->canIteratorDir());
}

TEST_F(TestMasteredMediaFileInfo, parentUrlIsRoot)
{
    //    EXPECT_STREQ(QDir("~/").absolutePath().toStdString().c_str(), info->parentUrl().path().toStdString().c_str());
    EXPECT_FALSE(info->parentUrl().path().isEmpty());
}

TEST_F(TestMasteredMediaFileInfo, goToUrlWhenDeleted)
{
    //    EXPECT_STREQ(QDir("~/").absolutePath().toStdString().c_str(), info->goToUrlWhenDeleted().path().toStdString().c_str());
    EXPECT_FALSE(info->parentUrl().path().isEmpty());
}

TEST_F(TestMasteredMediaFileInfo, toLocalFile)
{
    EXPECT_STREQ("", info->toLocalFile().toStdString().c_str());
}

TEST_F(TestMasteredMediaFileInfo, canDrop)
{
    EXPECT_TRUE(info->canDrop());
}

TEST_F(TestMasteredMediaFileInfo, canTag)
{
    EXPECT_FALSE(info->canTag());
}

TEST_F(TestMasteredMediaFileInfo, canRename)
{
    info->refresh();
    EXPECT_FALSE(info->canRename());
}
