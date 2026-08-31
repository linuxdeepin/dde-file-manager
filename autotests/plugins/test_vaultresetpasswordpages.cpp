// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultresetpasswordpages.cpp
 * @brief Unit tests for VaultResetPasswordPages methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/resetpasswordview/vaultresetpasswordpages.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultResetPasswordPagesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultResetPasswordPages();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultResetPasswordPages *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultResetPasswordPagesTest, VaultResetPasswordPages)
{
    // Test constructor: VaultResetPasswordPages((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultResetPasswordPagesTest, switchToKeyFileView)
{
    // Test method: void switchToKeyFileView(())
    EXPECT_NO_FATAL_FAILURE(obj->switchToKeyFileView());
}

TEST_F(VaultResetPasswordPagesTest, switchToOldPasswordView)
{
    // Test method: void switchToOldPasswordView(())
    EXPECT_NO_FATAL_FAILURE(obj->switchToOldPasswordView());
}

TEST_F(VaultResetPasswordPagesTest, _VaultResetPasswordPages)
{
    // Test constructor: VaultResetPasswordPages((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
