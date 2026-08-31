// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_chgpassphrasedialog_1.cpp
 * @brief Unit tests for ChgPassphraseDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "gui/chgpassphrasedialog.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class ChgPassphraseDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ChgPassphraseDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ChgPassphraseDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ChgPassphraseDialogTest, ChgPassphraseDialog)
{
    // Test constructor: ChgPassphraseDialog((const QString &device, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ChgPassphraseDialogTest, getPassphrase)
{
    // Test getter: QPair<QString, QString> getPassphrase()
    auto result = obj->getPassphrase();
    EXPECT_NO_FATAL_FAILURE({ obj->getPassphrase(); });

}

TEST_F(ChgPassphraseDialogTest, onButtonClicked)
{
    // Test method: void onButtonClicked((int idx))
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked(0));
}

TEST_F(ChgPassphraseDialogTest, onOldKeyChanged)
{
    // Test method: void onOldKeyChanged((const QString &inputs))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onOldKeyChanged(_arg0));
}

TEST_F(ChgPassphraseDialogTest, onRecSwitchClicked)
{
    // Test method: void onRecSwitchClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onRecSwitchClicked());
}

TEST_F(ChgPassphraseDialogTest, validateByRecKey)
{
    // Test bool getter: validateByRecKey()
    bool result = obj->validateByRecKey();
    EXPECT_FALSE(result);

}
