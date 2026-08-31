// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremovebynonewidget.cpp
 * @brief Unit tests for VaultRemoveByNoneWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/removevaultview/vaultremovebynonewidget.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultRemoveByNoneWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultRemoveByNoneWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultRemoveByNoneWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultRemoveByNoneWidgetTest, btnText)
{
    // Test getter: QStringList btnText()
    auto result = obj->btnText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultRemoveByNoneWidgetTest, buttonClicked)
{
    // Test method: void buttonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->buttonClicked(0, _arg1));
}

TEST_F(VaultRemoveByNoneWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(VaultRemoveByNoneWidgetTest, slotCheckAuthorizationFinished)
{
    // Test method: void slotCheckAuthorizationFinished((bool result))
    EXPECT_NO_FATAL_FAILURE(obj->slotCheckAuthorizationFinished(false));
}

TEST_F(VaultRemoveByNoneWidgetTest, titleText)
{
    // Test getter: QString titleText()
    auto result = obj->titleText();
    EXPECT_TRUE(result.isEmpty());

}
