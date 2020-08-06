#include "models/mergeddesktopfileinfo.h"

#include <gtest/gtest.h>

namespace {
class TestMergedDesktopFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMergedDesktopFileInfo";

        info = new MergedDesktopFileInfo(DUrl("/"), DUrl("/"));
    }

    void TearDown() override
    {
        std::cout << "end TestMergedDesktopFileInfo";
    }

public:
    MergedDesktopFileInfo *info;
};
} // namespace

TEST_F(TestMergedDesktopFileInfo, parentUrl)
{
    EXPECT_STREQ("/", info->parentUrl().path().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, iconName)
{
    EXPECT_STREQ("", info->iconName().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, genericIconName)
{
    EXPECT_STREQ("-x-generic", info->genericIconName().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, mimeDataUrl)
{
    EXPECT_STREQ("/", info->mimeDataUrl().path().toStdString().c_str());
}

TEST_F(TestMergedDesktopFileInfo, canRedirectUrl)
{
    EXPECT_FALSE(info->canRedirectionFileUrl());
}

TEST_F(TestMergedDesktopFileInfo, redirectedFileUrl)
{
    EXPECT_STREQ("/", info->redirectedFileUrl().path().toStdString().c_str());
}
