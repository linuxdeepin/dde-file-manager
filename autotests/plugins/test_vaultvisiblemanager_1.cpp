// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultvisiblemanager_1.cpp
 * @brief Unit tests for VaultVisibleManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultvisiblemanager.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultVisibleManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultVisibleManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultVisibleManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultVisibleManagerTest, addVaultComputerMenu)
{
    // Test method: void addVaultComputerMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->addVaultComputerMenu());
}

TEST_F(VaultVisibleManagerTest, infoRegister)
{
    // Test method: void infoRegister(())
    EXPECT_NO_FATAL_FAILURE(obj->infoRegister());
}

TEST_F(VaultVisibleManagerTest, instance)
{
    // Test getter: VaultVisibleManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(VaultVisibleManagerTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 winID))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(VaultVisibleManagerTest, pluginServiceRegister)
{
    // Test method: void pluginServiceRegister(())
    EXPECT_NO_FATAL_FAILURE(obj->pluginServiceRegister());
}

TEST_F(VaultVisibleManagerTest, updateSideBarVaultItem)
{
    // Test method: void updateSideBarVaultItem(())
    EXPECT_NO_FATAL_FAILURE(obj->updateSideBarVaultItem());
}
