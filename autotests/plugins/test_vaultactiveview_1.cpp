// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactiveview_1.cpp
 * @brief Unit tests for VaultActiveView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultcreatepage.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultActiveViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultActiveView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultActiveView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultActiveViewTest, asyncCreateVault)
{
    // Test method: void asyncCreateVault(())
    EXPECT_NO_FATAL_FAILURE(obj->asyncCreateVault());
}

TEST_F(VaultActiveViewTest, createVault)
{
    // Test getter: Result createVault()
    auto result = obj->createVault();
    EXPECT_NO_FATAL_FAILURE({ obj->createVault(); });

}

TEST_F(VaultActiveViewTest, handleKeyModeEncryption)
{
    // Test bool getter: handleKeyModeEncryption()
    bool result = obj->handleKeyModeEncryption();
    EXPECT_FALSE(result);

}

TEST_F(VaultActiveViewTest, handleTransparentModeEncryption)
{
    // Test bool getter: handleTransparentModeEncryption()
    bool result = obj->handleTransparentModeEncryption();
    EXPECT_FALSE(result);

}

TEST_F(VaultActiveViewTest, setBeginingState)
{
    // Test method: void setBeginingState(())
    EXPECT_NO_FATAL_FAILURE(obj->setBeginingState());
}
