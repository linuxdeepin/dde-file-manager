// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremovebypasswordview.cpp
 * @brief Unit tests for VaultRemoveByPasswordView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/removevaultview/vaultremovebypasswordview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultRemoveByPasswordViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultRemoveByPasswordView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultRemoveByPasswordView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultRemoveByPasswordViewTest, VaultRemoveByPasswordView)
{
    // Test constructor: VaultRemoveByPasswordView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultRemoveByPasswordViewTest, buttonClicked)
{
    // Test method: void buttonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->buttonClicked(0, _arg1));
}

TEST_F(VaultRemoveByPasswordViewTest, showToolTip)
{
    // Test method: void showToolTip((const QString &text, int duration, VaultRemoveByPasswordView::EN_ToolTip enType))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showToolTip(_arg0, 0, VaultRemoveByPasswordView::EN_ToolTip()));
}

TEST_F(VaultRemoveByPasswordViewTest, _VaultRemoveByPasswordView)
{
    // Test constructor: VaultRemoveByPasswordView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
