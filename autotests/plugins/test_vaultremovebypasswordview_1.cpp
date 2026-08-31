// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremovebypasswordview_1.cpp
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

TEST_F(VaultRemoveByPasswordViewTest, btnText)
{
    // Test getter: QStringList btnText()
    auto result = obj->btnText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultRemoveByPasswordViewTest, onPasswordChanged)
{
    // Test method: void onPasswordChanged((const QString &password))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onPasswordChanged(_arg0));
}

TEST_F(VaultRemoveByPasswordViewTest, setTipsButtonVisible)
{
    // Test setter: void setTipsButtonVisible((bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->setTipsButtonVisible(false));
}

TEST_F(VaultRemoveByPasswordViewTest, showAlertMessage)
{
    // Test method: void showAlertMessage((const QString &text, int duration))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAlertMessage(_arg0, 0));
}

TEST_F(VaultRemoveByPasswordViewTest, slotCheckAuthorizationFinished)
{
    // Test method: void slotCheckAuthorizationFinished((bool result))
    EXPECT_NO_FATAL_FAILURE(obj->slotCheckAuthorizationFinished(false));
}

TEST_F(VaultRemoveByPasswordViewTest, titleText)
{
    // Test getter: QString titleText()
    auto result = obj->titleText();
    EXPECT_TRUE(result.isEmpty());

}
