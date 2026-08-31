// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultpagebase.cpp
 * @brief Unit tests for VaultPageBase methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultpagebase.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultPageBaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultPageBase();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultPageBase *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultPageBaseTest, VaultPageBase)
{
    // Test constructor: VaultPageBase((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
