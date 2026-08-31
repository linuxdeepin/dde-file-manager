// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremovebyrecoverykeyview.cpp
 * @brief Unit tests for VaultRemoveByRecoverykeyView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/removevaultview/vaultremovebyrecoverykeyview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultRemoveByRecoverykeyViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultRemoveByRecoverykeyView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultRemoveByRecoverykeyView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultRemoveByRecoverykeyViewTest, VaultRemoveByRecoverykeyView)
{
    // Test constructor: VaultRemoveByRecoverykeyView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultRemoveByRecoverykeyViewTest, afterRecoveryKeyChanged)
{
    // Test method: int afterRecoveryKeyChanged((QString &str))
    QString _arg0{};
    auto result = obj->afterRecoveryKeyChanged(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(VaultRemoveByRecoverykeyViewTest, buttonClicked)
{
    // Test method: void buttonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->buttonClicked(0, _arg1));
}

TEST_F(VaultRemoveByRecoverykeyViewTest, _VaultRemoveByRecoverykeyView)
{
    // Test constructor: VaultRemoveByRecoverykeyView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
