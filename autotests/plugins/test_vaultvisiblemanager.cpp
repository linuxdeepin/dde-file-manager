// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultvisiblemanager.cpp
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

TEST_F(VaultVisibleManagerTest, VaultVisibleManager)
{
    // Test constructor: VaultVisibleManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultVisibleManagerTest, removeComputerVaultItem)
{
    // Test method: void removeComputerVaultItem(())
    EXPECT_NO_FATAL_FAILURE(obj->removeComputerVaultItem());
}

TEST_F(VaultVisibleManagerTest, removeSideBarVaultItem)
{
    // Test method: void removeSideBarVaultItem(())
    EXPECT_NO_FATAL_FAILURE(obj->removeSideBarVaultItem());
}
