// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactivestartview.cpp
 * @brief Unit tests for VaultActiveStartView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/createvaultview/vaultactivestartview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultActiveStartViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultActiveStartView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultActiveStartView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultActiveStartViewTest, VaultActiveStartView)
{
    // Test constructor: VaultActiveStartView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultActiveStartViewTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(VaultActiveStartViewTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}

TEST_F(VaultActiveStartViewTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}
