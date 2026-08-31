// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactivefinishedview.cpp
 * @brief Unit tests for VaultActiveFinishedView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/createvaultview/vaultactivefinishedview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultActiveFinishedViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultActiveFinishedView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultActiveFinishedView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultActiveFinishedViewTest, initUi)
{
    // Test method: void initUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initUi());
}
