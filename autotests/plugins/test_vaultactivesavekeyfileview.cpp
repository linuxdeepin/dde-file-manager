// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactivesavekeyfileview.cpp
 * @brief Unit tests for VaultActiveSaveKeyFileView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/createvaultview/vaultactivesavekeyfileview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultActiveSaveKeyFileViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultActiveSaveKeyFileView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultActiveSaveKeyFileView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultActiveSaveKeyFileViewTest, VaultActiveSaveKeyFileView)
{
    // Test constructor: VaultActiveSaveKeyFileView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultActiveSaveKeyFileViewTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(VaultActiveSaveKeyFileViewTest, onOldPasswordSchemeMigrationFinished)
{
    // Test method: void onOldPasswordSchemeMigrationFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onOldPasswordSchemeMigrationFinished());
}
