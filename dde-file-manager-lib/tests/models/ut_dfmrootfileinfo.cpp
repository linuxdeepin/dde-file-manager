#include "models/dfmrootfileinfo.h"
#include "interfaces/dfmstandardpaths.h"

#include <gtest/gtest.h>

namespace {
class TestDFMRootFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMRootFileInfo";
        info = new DFMRootFileInfo(DUrl("dfmroot:///desktop.userdir"));
    }

    void TearDown() override
    {
        std::cout << "end TestDFMRootFileInfo";
//        delete info;
    }

public:
    DFMRootFileInfo *info;
};
} // namespace

TEST_F(TestDFMRootFileInfo, exist)
{
    ASSERT_TRUE(info->exists());
}

TEST_F(TestDFMRootFileInfo, canRename)
{
    ASSERT_FALSE(info->canRename());
}

TEST_F(TestDFMRootFileInfo, suffix)
{
    EXPECT_STREQ("userdir", info->suffix().toStdString().c_str());
}

TEST_F(TestDFMRootFileInfo, fileDisplayName)
{
    EXPECT_STREQ("Desktop", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestDFMRootFileInfo, canShare)
{
    ASSERT_FALSE(info->canShare());
}

TEST_F(TestDFMRootFileInfo, canFetch)
{
    ASSERT_FALSE(info->canFetch());
}

TEST_F(TestDFMRootFileInfo, isReadable)
{
    ASSERT_TRUE(info->isReadable());
}

TEST_F(TestDFMRootFileInfo, isWritable)
{
    ASSERT_FALSE(info->isWritable());
}

TEST_F(TestDFMRootFileInfo, isExecutable)
{
    ASSERT_FALSE(info->isExecutable());
}

TEST_F(TestDFMRootFileInfo, isHidden)
{
    ASSERT_FALSE(info->isHidden());
}

TEST_F(TestDFMRootFileInfo, isRelative)
{
    ASSERT_FALSE(info->isRelative());
}

TEST_F(TestDFMRootFileInfo, isAbsolute)
{
    ASSERT_TRUE(info->isAbsolute());
}

TEST_F(TestDFMRootFileInfo, isShared)
{
    ASSERT_FALSE(info->isShared());
}

TEST_F(TestDFMRootFileInfo, isTaged)
{
    ASSERT_FALSE(info->isTaged());
}

TEST_F(TestDFMRootFileInfo, isWritableShared)
{
    ASSERT_FALSE(info->isWritableShared());
}

TEST_F(TestDFMRootFileInfo, isAllowGuestShared)
{
    ASSERT_FALSE(info->isAllowGuestShared());
}

TEST_F(TestDFMRootFileInfo, isFile)
{
    ASSERT_TRUE(info->isFile());
}

TEST_F(TestDFMRootFileInfo, isDir)
{
    ASSERT_FALSE(info->isDir());
}

TEST_F(TestDFMRootFileInfo, fileType)
{
    EXPECT_EQ(257, info->fileType());
}

TEST_F(TestDFMRootFileInfo, filesCount)
{
    EXPECT_EQ(0, info->filesCount());
}

TEST_F(TestDFMRootFileInfo, iconName)
{
    EXPECT_STREQ("user-desktop", info->iconName().toStdString().c_str());
}

TEST_F(TestDFMRootFileInfo, canRedirectionFileUrl)
{
    EXPECT_TRUE(info->canRedirectionFileUrl());
}

TEST_F(TestDFMRootFileInfo, redirectedFileUrl)
{
    EXPECT_STREQ(DFMStandardPaths::location(DFMStandardPaths::DesktopPath).toStdString().c_str(), info->redirectedFileUrl().path().toStdString().c_str());
}

TEST_F(TestDFMRootFileInfo, canDrop)
{
    EXPECT_FALSE(info->canDrop());
}

TEST_F(TestDFMRootFileInfo, tstSupportedDragNDropActions)
{
    EXPECT_TRUE(Qt::DropAction::IgnoreAction == info->supportedDragActions());
    EXPECT_TRUE(Qt::DropAction::IgnoreAction == info->supportedDropActions());
}

TEST_F(TestDFMRootFileInfo, tstGetVoltag)
{
    EXPECT_TRUE(!info->getVolTag().isEmpty());
}


TEST_F(TestDFMRootFileInfo, tstCheckMpsStr)
{
    EXPECT_FALSE(info->checkMpsStr("Test"));
}

TEST_F(TestDFMRootFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(info->menuActionList().count() > 0);
}
