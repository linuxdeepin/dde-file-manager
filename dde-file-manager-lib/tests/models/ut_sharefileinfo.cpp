#include "models/sharefileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestShareFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestShareFileInfo";
        info = new ShareFileInfo(DUrl("file:///test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestShareFileInfo";
    }

public:
    ShareFileInfo *info;
};
} // namespace

//TEST_F(TestShareFileInfo, fileExist)
//{
//    EXPECT_TRUE(info->exists());
//}

//TEST_F(TestShareFileInfo, fileIsDir)
//{
//    EXPECT_TRUE(info->isDir());
//}

//TEST_F(TestShareFileInfo, fileCanRename)
//{
//    EXPECT_TRUE(info->canRename());
//}

//TEST_F(TestShareFileInfo, fileIsReadable)
//{
//    EXPECT_TRUE(info->isReadable());
//}

//TEST_F(TestShareFileInfo, fileIsWritable)
//{
//    EXPECT_TRUE(info->isWritable());
//}

//TEST_F(TestShareFileInfo, fileDisplayName)
//{
//    EXPECT_STREQ("test", info->fileDisplayName().toStdString().c_str());
//}

//TEST_F(TestShareFileInfo, fileCanIterator)
//{
//    EXPECT_TRUE(info->canIteratorDir());
//}

//TEST_F(TestShareFileInfo, makeAbsolute)
//{
//    EXPECT_TRUE(info->makeAbsolute());
//}

//TEST_F(TestShareFileInfo, isShared)
//{
//    EXPECT_TRUE(info->isShared());
//}

//TEST_F(TestShareFileInfo, canTag)
//{
//    EXPECT_TRUE(info->canTag());
//}

//TEST_F(TestShareFileInfo, isVirtualEntry)
//{
//    EXPECT_TRUE(info->isVirtualEntry());
//}

//TEST_F(TestShareFileInfo, canDrop)
//{
//    EXPECT_TRUE(info->canDrop());
//}
