#include <gtest/gtest.h>
#include "models/tagfileinfo.h"
#include "interfaces/dfmstandardpaths.h"

namespace {
class TestTagFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestTagFile";
        info = new TagFileInfo(DUrl("tag:///"));
    }

    void TearDown() override
    {
        std::cout << "end TestTagFile";
        delete info;
        info = nullptr;
    }

public:
    TagFileInfo *info;
};
} // namespace

TEST_F(TestTagFileInfo, PropertyTest)
{
    EXPECT_TRUE(info->isDir());
    EXPECT_TRUE(info->makeAbsolute());
    EXPECT_TRUE(info->exists());
    EXPECT_FALSE(info->canRename());
    EXPECT_TRUE(info->isTaged());
    EXPECT_TRUE(info->isWritable());
    EXPECT_FALSE(info->canRedirectionFileUrl());
    EXPECT_TRUE(info->canIteratorDir());
    EXPECT_FALSE(info->canDrop());
    EXPECT_TRUE(info->isVirtualEntry());
}

TEST_F(TestTagFileInfo, UrlTest)
{
    EXPECT_STREQ("", info->parentUrl().path().toStdString().c_str());
    EXPECT_STREQ(DFMStandardPaths::location(DFMStandardPaths::HomePath).toStdString().c_str(),
                 info->goToUrlWhenDeleted().path().toStdString().c_str());
    EXPECT_STREQ("/", info->redirectedFileUrl().path().toStdString().c_str());
    EXPECT_STREQ("", info->mimeDataUrl().path().toStdString().c_str());
}

TEST_F(TestTagFileInfo, StringPropertyTest)
{
    EXPECT_STREQ("folder", info->iconName().toStdString().c_str());
    EXPECT_STREQ("-", info->sizeDisplayName().toStdString().c_str());
}
