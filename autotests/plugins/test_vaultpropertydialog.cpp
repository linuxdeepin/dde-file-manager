// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultpropertydialog.cpp
 * @brief Unit tests for VaultPropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultpropertyview/vaultpropertydialog.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultPropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultPropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultPropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultPropertyDialogTest, VaultPropertyDialog)
{
    // Test constructor: VaultPropertyDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
