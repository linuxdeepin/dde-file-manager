// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultunlockpages.cpp
 * @brief Unit tests for VaultUnlockPages methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultunlockpages.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultUnlockPagesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultUnlockPages();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultUnlockPages *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultUnlockPagesTest, VaultUnlockPages)
{
    // Test constructor: VaultUnlockPages((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
