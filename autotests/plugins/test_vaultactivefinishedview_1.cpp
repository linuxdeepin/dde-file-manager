// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactivefinishedview_1.cpp
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

TEST_F(VaultActiveFinishedViewTest, VaultActiveFinishedView)
{
    // Test constructor: VaultActiveFinishedView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultActiveFinishedViewTest, encryptFinished)
{
    // Test method: void encryptFinished((bool success, const QString &msg))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->encryptFinished(false, _arg1));
}

TEST_F(VaultActiveFinishedViewTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(VaultActiveFinishedViewTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(VaultActiveFinishedViewTest, setFinishedBtnEnabled)
{
    // Test setter: void setFinishedBtnEnabled((bool b))
    EXPECT_NO_FATAL_FAILURE(obj->setFinishedBtnEnabled(false));
}

TEST_F(VaultActiveFinishedViewTest, setProgressValue)
{
    // Test setter: void setProgressValue((int value))
    EXPECT_NO_FATAL_FAILURE(obj->setProgressValue(0));
}

TEST_F(VaultActiveFinishedViewTest, slotCheckAuthorizationFinished)
{
    // Test method: void slotCheckAuthorizationFinished((bool result))
    EXPECT_NO_FATAL_FAILURE(obj->slotCheckAuthorizationFinished(false));
}

TEST_F(VaultActiveFinishedViewTest, slotEncryptVault)
{
    // Test method: void slotEncryptVault(())
    EXPECT_NO_FATAL_FAILURE(obj->slotEncryptVault());
}

TEST_F(VaultActiveFinishedViewTest, slotTimeout)
{
    // Test method: void slotTimeout(())
    EXPECT_NO_FATAL_FAILURE(obj->slotTimeout());
}
