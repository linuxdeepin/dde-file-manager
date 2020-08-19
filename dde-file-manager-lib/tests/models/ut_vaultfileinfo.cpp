#include <gtest/gtest.h>
#include "models/vaultfileinfo.h"

namespace {
class TestVaultFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestVaultFileInfo";
        info = new VaultFileInfo(DUrl("dfmvault:///"));
    }

    void TearDown() override
    {
        std::cout << "end TestVaultFileInfo";
        delete info;
        info = nullptr;
    }

public:
    VaultFileInfo *info;
};
} // namespace

TEST_F(TestVaultFileInfo, BoolPropertyTest)
{
    EXPECT_TRUE(info->exists());
    EXPECT_FALSE(info->canRename());
    EXPECT_TRUE(info->isReadable());
    EXPECT_TRUE(info->isWritable());
    EXPECT_FALSE(info->canShare());
    EXPECT_TRUE(info->isDir());
    EXPECT_TRUE(info->canIteratorDir());
    EXPECT_FALSE(info->makeAbsolute());
    EXPECT_FALSE(info->canRedirectionFileUrl());
    EXPECT_FALSE(info->isSymLink());
    EXPECT_FALSE(info->canTag());
    EXPECT_FALSE(info->canDrop());
}

TEST_F(TestVaultFileInfo, UrlTest)
{
    EXPECT_STREQ("", info->parentUrl().path().toStdString().c_str());
    EXPECT_STREQ("/", info->goToUrlWhenDeleted().path().toStdString().c_str());
    EXPECT_TRUE(info->redirectedFileUrl().path().contains(".local/share/applications/vault_unlocked/"));
    EXPECT_TRUE(info->mimeDataUrl().path().contains(".local/share/applications/vault_unlocked"));
}

TEST_F(TestVaultFileInfo, StringPropertyTest)
{
    EXPECT_STREQ("My Vault", info->fileDisplayName().toStdString().c_str());
    EXPECT_STREQ("Folder is empty", info->subtitleForEmptyFloder().toStdString().c_str());
    EXPECT_STREQ("dfm_safebox", info->iconName().toStdString().c_str());
    EXPECT_STREQ("folder", info->genericIconName().toStdString().c_str());
}
