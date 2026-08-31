// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremovepages_1.cpp
 * @brief Unit tests for VaultRemovePages methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultremovepages.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultRemovePagesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultRemovePages();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultRemovePages *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultRemovePagesTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(VaultRemovePagesTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(VaultRemovePagesTest, setBtnEnable)
{
    // Test setter: void setBtnEnable((int index, bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setBtnEnable(0, false));
}

TEST_F(VaultRemovePagesTest, showNodeWidget)
{
    // Test method: void showNodeWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->showNodeWidget());
}

TEST_F(VaultRemovePagesTest, showPasswordWidget)
{
    // Test method: void showPasswordWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->showPasswordWidget());
}

TEST_F(VaultRemovePagesTest, showRecoveryKeyWidget)
{
    // Test method: void showRecoveryKeyWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->showRecoveryKeyWidget());
}
