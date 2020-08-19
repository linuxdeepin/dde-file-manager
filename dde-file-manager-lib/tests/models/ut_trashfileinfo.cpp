#include <gtest/gtest.h>
#include "models/trashfileinfo.h"

namespace {
class TestTrashFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestTrashFileInfo";
        info = new TrashFileInfo(DUrl("trash:///"));
    }

    void TearDown() override
    {
        std::cout << "end TestTrashFileInfo";
        delete info;
        info = nullptr;
    }

public:
    TrashFileInfo *info;
};
} // namespace

TEST_F(TestTrashFileInfo, BoolPropertyTest)
{
    EXPECT_TRUE(info->exists());
    EXPECT_FALSE(info->canRename());
    EXPECT_TRUE(info->isReadable());
    EXPECT_TRUE(info->isWritable());
    EXPECT_FALSE(info->canShare());
    EXPECT_TRUE(info->isDir());
    EXPECT_TRUE(info->canIteratorDir());
    EXPECT_TRUE(info->makeAbsolute());
    EXPECT_FALSE(info->restore());
}

TEST_F(TestTrashFileInfo, UrlTest)
{
    EXPECT_STREQ("", info->originUrl().path().toStdString().c_str());
    EXPECT_STREQ("/", info->goToUrlWhenDeleted().path().toStdString().c_str());
    EXPECT_TRUE(info->mimeDataUrl().path().contains(".local/share/Trash/files"));
}

TEST_F(TestTrashFileInfo, StringPropertyTest)
{
    EXPECT_STREQ("Trash", info->fileDisplayName().toStdString().c_str());
    EXPECT_STREQ("Folder is empty", info->subtitleForEmptyFloder().toStdString().c_str());
    EXPECT_STREQ("", info->sourceFilePath().toStdString().c_str());
}
