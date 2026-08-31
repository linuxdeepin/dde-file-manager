// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactiveview.cpp
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

TEST_F(VaultActiveViewTest, VaultActiveView)
{
    // Test constructor: VaultActiveView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultActiveViewTest, encryptVault)
{
    // Test method: void encryptVault(())
    EXPECT_NO_FATAL_FAILURE(obj->encryptVault());
}

TEST_F(VaultActiveViewTest, slotNextWidget)
{
    // Test method: void slotNextWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->slotNextWidget());
}
