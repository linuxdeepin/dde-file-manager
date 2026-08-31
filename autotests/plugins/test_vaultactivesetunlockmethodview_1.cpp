// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactivesetunlockmethodview_1.cpp
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

TEST_F(VaultActiveSetUnlockMethodViewTest, VaultActiveSetUnlockMethodView)
{
    // Test constructor: VaultActiveSetUnlockMethodView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultActiveSetUnlockMethodViewTest, checkInputInfo)
{
    // Test bool getter: checkInputInfo()
    bool result = obj->checkInputInfo();
    EXPECT_FALSE(result);

}

TEST_F(VaultActiveSetUnlockMethodViewTest, checkPassword)
{
    // Test method: bool checkPassword((const QString &password))
    QString _arg0{};
    auto result = obj->checkPassword(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(VaultActiveSetUnlockMethodViewTest, checkRepeatPassword)
{
    // Test bool getter: checkRepeatPassword()
    bool result = obj->checkRepeatPassword();
    EXPECT_FALSE(result);

}

TEST_F(VaultActiveSetUnlockMethodViewTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(VaultActiveSetUnlockMethodViewTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(VaultActiveSetUnlockMethodViewTest, setEncryptInfo)
{
    // Test setter: void setEncryptInfo((EncryptInfo &info))
    EncryptInfo _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEncryptInfo(_arg0));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotGenerateEditChanged)
{
    // Test method: void slotGenerateEditChanged((const QString &str))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotGenerateEditChanged(_arg0));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotLimiPasswordLength)
{
    // Test method: void slotLimiPasswordLength((const QString &password))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotLimiPasswordLength(_arg0));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotPasswordEditFinished)
{
    // Test method: void slotPasswordEditFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->slotPasswordEditFinished());
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotPasswordEditFocusChanged)
{
    // Test method: void slotPasswordEditFocusChanged((bool bFocus))
    EXPECT_NO_FATAL_FAILURE(obj->slotPasswordEditFocusChanged(false));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotPasswordEditing)
{
    // Test method: void slotPasswordEditing(())
    EXPECT_NO_FATAL_FAILURE(obj->slotPasswordEditing());
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotRepeatPasswordEditFinished)
{
    // Test method: void slotRepeatPasswordEditFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->slotRepeatPasswordEditFinished());
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotRepeatPasswordEditFocusChanged)
{
    // Test method: void slotRepeatPasswordEditFocusChanged((bool bFocus))
    EXPECT_NO_FATAL_FAILURE(obj->slotRepeatPasswordEditFocusChanged(false));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotRepeatPasswordEditing)
{
    // Test method: void slotRepeatPasswordEditing(())
    EXPECT_NO_FATAL_FAILURE(obj->slotRepeatPasswordEditing());
}

TEST_F(VaultActiveSetUnlockMethodViewTest, slotTypeChanged)
{
    // Test method: void slotTypeChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->slotTypeChanged(0));
}

TEST_F(VaultActiveSetUnlockMethodViewTest, VaultActiveSetUnlockMethodView_Destructor)
{
    // Test method:  ~VaultActiveSetUnlockMethodView(())
    EXPECT_NO_FATAL_FAILURE({ VaultActiveSetUnlockMethodView *tmp = new VaultActiveSetUnlockMethodView(); delete tmp; });
}
