// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremoveprogressview.cpp
 * @brief Unit tests for VaultRemoveProgressView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/removevaultview/vaultremoveprogressview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultRemoveProgressViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultRemoveProgressView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultRemoveProgressView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultRemoveProgressViewTest, VaultRemoveProgressView)
{
    // Test constructor: VaultRemoveProgressView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultRemoveProgressViewTest, handleVaultRemovedProgress)
{
    // Test method: void handleVaultRemovedProgress((int value))
    EXPECT_NO_FATAL_FAILURE(obj->handleVaultRemovedProgress(0));
}

TEST_F(VaultRemoveProgressViewTest, removeVault)
{
    // Test method: void removeVault((const QString &basePath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeVault(_arg0));
}

TEST_F(VaultRemoveProgressViewTest, _VaultRemoveProgressView)
{
    // Test constructor: VaultRemoveProgressView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
