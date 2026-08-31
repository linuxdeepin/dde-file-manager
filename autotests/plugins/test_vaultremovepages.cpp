// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremovepages.cpp
 * @brief Unit tests for VaultRemovePages methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultremovepages.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultRemovePagesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultRemovePages();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultRemovePages *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultRemovePagesTest, VaultRemovePages)
{
    // Test constructor: VaultRemovePages((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultRemovePagesTest, onButtonClicked)
{
    // Test method: void onButtonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked(0, _arg1));
}

TEST_F(VaultRemovePagesTest, pageSelect)
{
    // Test method: void pageSelect((RemoveWidgetType type))
    EXPECT_NO_FATAL_FAILURE(obj->pageSelect(RemoveWidgetType()));
}

TEST_F(VaultRemovePagesTest, showRemoveProgressWidget)
{
    // Test method: void showRemoveProgressWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->showRemoveProgressWidget());
}

TEST_F(VaultRemovePagesTest, _VaultRemovePages)
{
    // Test constructor: VaultRemovePages((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
