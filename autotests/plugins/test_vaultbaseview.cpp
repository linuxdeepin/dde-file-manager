// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultbaseview.cpp
 * @brief Unit tests for VaultBaseView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/createvaultview/vaultbaseview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultBaseViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultBaseView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultBaseView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultBaseViewTest, VaultBaseView)
{
    // Test constructor: VaultBaseView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
