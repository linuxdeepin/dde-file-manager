/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-28

 */


#include <gtest/gtest.h>

#include "durl.h"

#define private public

#include "models/vaultfileinfo.h"

namespace  {
class TestVaultFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        m_url = DUrl("dfmvault:///" + QDir::homePath() + "/.local/share/applications");
        m_vaultFileInfo = new VaultFileInfo(m_url);
    }

    void TearDown() override
    {
        delete m_vaultFileInfo;
        m_vaultFileInfo = nullptr;
    }

public:
    VaultFileInfo * m_vaultFileInfo;
    DUrl m_url;
};
}

TEST_F(TestVaultFileInfo, get_exists)
{
    EXPECT_TRUE(m_vaultFileInfo->exists());
}

TEST_F(TestVaultFileInfo, get_parent_url)
{
    EXPECT_FALSE(m_vaultFileInfo->parentUrl().isValid());
}

TEST_F(TestVaultFileInfo, get_icon_name)
{
    EXPECT_TRUE(!m_vaultFileInfo->iconName().isEmpty());
}

TEST_F(TestVaultFileInfo, get_generic_icon_name)
{
    EXPECT_TRUE(!m_vaultFileInfo->genericIconName().isEmpty());
}

TEST_F(TestVaultFileInfo, get_mime_data_url)
{
    EXPECT_TRUE(m_vaultFileInfo->mimeDataUrl().isValid());
}

TEST_F(TestVaultFileInfo, can_redirection_file_url)
{
    EXPECT_FALSE(m_vaultFileInfo->canRedirectionFileUrl());
}

TEST_F(TestVaultFileInfo, get_redirected_file_url)
{
    EXPECT_TRUE(m_vaultFileInfo->redirectedFileUrl().isValid());
}

TEST_F(TestVaultFileInfo, can_iterator_dir)
{
    EXPECT_TRUE(m_vaultFileInfo->canIteratorDir());
}

TEST_F(TestVaultFileInfo, get_subtitle_for_empty_floder)
{
    EXPECT_TRUE(!m_vaultFileInfo->subtitleForEmptyFloder().isEmpty());
}

TEST_F(TestVaultFileInfo, get_url_bynew_file_name)
{
    EXPECT_TRUE(m_vaultFileInfo->getUrlByNewFileName(QString("applications")).isValid());
}

TEST_F(TestVaultFileInfo, get_additional_icon)
{
    EXPECT_FALSE(!m_vaultFileInfo->additionalIcon().isEmpty());
}

TEST_F(TestVaultFileInfo, can_is_Writable)
{
    EXPECT_TRUE(m_vaultFileInfo->isWritable());
}

TEST_F(TestVaultFileInfo, can_is_symLink)
{
    EXPECT_FALSE(m_vaultFileInfo->isSymLink());
}

TEST_F(TestVaultFileInfo, get_permissions)
{
    EXPECT_TRUE(m_vaultFileInfo->permissions());
}

TEST_F(TestVaultFileInfo, get_disable_menu_action_list)
{
    EXPECT_FALSE(!m_vaultFileInfo->disableMenuActionList().isEmpty());
}

TEST_F(TestVaultFileInfo, get_menu_action_list)
{
    EXPECT_TRUE(!m_vaultFileInfo->menuActionList(DAbstractFileInfo::SingleFile).isEmpty());
}

TEST_F(TestVaultFileInfo, get_sub_menu_action_list)
{
    EXPECT_TRUE(!m_vaultFileInfo->subMenuActionList().isEmpty());
}

TEST_F(TestVaultFileInfo, get_file_display_name)
{
    EXPECT_TRUE(!m_vaultFileInfo->fileDisplayName().isEmpty());
}

TEST_F(TestVaultFileInfo, can_rename)
{
    EXPECT_TRUE(m_vaultFileInfo->canRename());
}

TEST_F(TestVaultFileInfo, can_share)
{
    EXPECT_FALSE(m_vaultFileInfo->canShare());
}

TEST_F(TestVaultFileInfo, can_tag)
{
    EXPECT_FALSE(m_vaultFileInfo->canTag());
}

TEST_F(TestVaultFileInfo, get_file_icon)
{
    EXPECT_FALSE(m_vaultFileInfo->fileIcon().isNull());
}

TEST_F(TestVaultFileInfo, get_size)
{
    EXPECT_TRUE(m_vaultFileInfo->size());
}

TEST_F(TestVaultFileInfo, get_is_dir)
{
    EXPECT_TRUE(m_vaultFileInfo->isDir());
}

TEST_F(TestVaultFileInfo, can_drop)
{
    EXPECT_FALSE(m_vaultFileInfo->canDrop());
}

TEST_F(TestVaultFileInfo, get_is_ancestors_url)
{
    DUrl url ("dfmvault:///" + QDir::homePath() + "/.local/share/applications");
    QList<DUrl> urlList;
    EXPECT_FALSE(m_vaultFileInfo->isAncestorsUrl(url, &urlList));
}

TEST_F(TestVaultFileInfo, get_is_root_directory)
{
    EXPECT_FALSE(m_vaultFileInfo->isRootDirectory());
}

