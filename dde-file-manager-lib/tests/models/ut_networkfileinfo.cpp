#include "models/networkfileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestNetworkFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestNetworkFileInfo";
        info = new NetworkFileInfo(DUrl("file:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestNetworkFileInfo";
    }

public:
    NetworkFileInfo *info;
};
} // namespace

TEST_F(TestNetworkFileInfo, filePath)
{
    EXPECT_STREQ("", info->filePath().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, absoluteFilePath)
{
    EXPECT_STREQ("", info->absoluteFilePath().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, isFileExists)
{
    EXPECT_TRUE(info->exists());
}

TEST_F(TestNetworkFileInfo, isFileReadable)
{
    EXPECT_TRUE(info->isReadable());
}

TEST_F(TestNetworkFileInfo, isFileWritable)
{
    EXPECT_TRUE(info->isWritable());
}

TEST_F(TestNetworkFileInfo, isFileVirtualEntry)
{
    EXPECT_FALSE(info->isVirtualEntry());
}

TEST_F(TestNetworkFileInfo, canFileDrop)
{
    EXPECT_TRUE(info->canDrop());
}

TEST_F(TestNetworkFileInfo, canFileRename)
{
    EXPECT_FALSE(info->canRename());
}

TEST_F(TestNetworkFileInfo, canFileIterator)
{
    EXPECT_FALSE(info->canIteratorDir());
}

TEST_F(TestNetworkFileInfo, fileIsDir)
{
    EXPECT_TRUE(info->isDir());
}

TEST_F(TestNetworkFileInfo, parentUrl)
{
    EXPECT_STREQ("", info->parentUrl().path().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, fileDisplayName)
{
    EXPECT_STREQ("", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, filesCount)
{
    EXPECT_EQ(-1, info->filesCount());
}

TEST_F(TestNetworkFileInfo, iconName)
{
    EXPECT_STREQ("", info->iconName().toStdString().c_str());
}

TEST_F(TestNetworkFileInfo, canRedirectUrl)
{
    EXPECT_TRUE(info->canRedirectionFileUrl());
}

TEST_F(TestNetworkFileInfo, redirectedUrl)
{
    EXPECT_STREQ("", info->redirectedFileUrl().path().toStdString().c_str());
}
