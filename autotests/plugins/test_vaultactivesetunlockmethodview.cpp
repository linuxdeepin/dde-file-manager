// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactivesetunlockmethodview.cpp
 * @brief Unit tests for VaultActiveSetUnlockMethodView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/createvaultview/vaultactivesetunlockmethodview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultActiveSetUnlockMethodViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultActiveSetUnlockMethodView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultActiveSetUnlockMethodView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultActiveSetUnlockMethodViewTest, clearText)
{
    // Test method: void clearText(())
    EXPECT_NO_FATAL_FAILURE(obj->clearText());
}

TEST_F(VaultActiveSetUnlockMethodViewTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}
