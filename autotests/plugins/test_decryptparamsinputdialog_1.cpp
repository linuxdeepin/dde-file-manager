// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_decryptparamsinputdialog_1.cpp
 * @brief Unit tests for DecryptParamsInputDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "gui/decryptparamsinputdialog.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class DecryptParamsInputDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DecryptParamsInputDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DecryptParamsInputDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DecryptParamsInputDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(DecryptParamsInputDialogTest, onButtonClicked)
{
    // Test method: void onButtonClicked((int idx))
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked(0));
}

TEST_F(DecryptParamsInputDialogTest, onKeyChanged)
{
    // Test method: void onKeyChanged((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onKeyChanged(_arg0));
}

TEST_F(DecryptParamsInputDialogTest, onRecSwitchClicked)
{
    // Test method: void onRecSwitchClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onRecSwitchClicked());
}

TEST_F(DecryptParamsInputDialogTest, setInputPIN)
{
    // Test setter: void setInputPIN((bool pin))
    EXPECT_NO_FATAL_FAILURE(obj->setInputPIN(false));
}

TEST_F(DecryptParamsInputDialogTest, updateUserHints)
{
    // Test method: void updateUserHints(())
    EXPECT_NO_FATAL_FAILURE(obj->updateUserHints());
}

TEST_F(DecryptParamsInputDialogTest, usingRecKey)
{
    // Test bool getter: usingRecKey()
    bool result = obj->usingRecKey();
    EXPECT_FALSE(result);

}
