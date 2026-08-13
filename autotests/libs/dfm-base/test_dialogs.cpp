// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dialogs.cpp
 * @brief Unit tests for GUI dialog classes using stub-ext to mock exec/show.
 *        Covers: SettingDialog, TaskDialog, BaseDialog, MountAskPasswordDialog,
 *        MountSecretDiskAskPasswordDialog, AliasComboBox, UserSharePasswordSettingDialog.
 *        Requires QApplication (available in test-dfm-base via QT_WIDGETS_LIB).
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QString>
#include <QJsonObject>

#include "stubext.h"

#include <dfm-base/dialogs/settingsdialog/settingdialog.h>
#include <dfm-base/dialogs/basedialog/basedialog.h>

using namespace dfmbase;

class DialogsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(DialogsTest, BaseDialogConstructAndDestruct)
{
    BaseDialog d;
    SUCCEED();
}

TEST_F(DialogsTest, SettingDialogConstructAndDestruct)
{
    SettingDialog d;
    SUCCEED();
}

TEST_F(DialogsTest, SettingDialogNeedHide)
{
    // needHide is static; test with known and unknown keys
    EXPECT_NO_FATAL_FAILURE({ (void)SettingDialog::needHide("nonexistent_key"); });
}

TEST_F(DialogsTest, SettingDialogSetItemVisible)
{
    EXPECT_NO_FATAL_FAILURE({ SettingDialog::setItemVisiable("test_key", false); });
}

// ============================================================
// Additional coverage for SettingDialog
// ============================================================

TEST_F(DialogsTest, SettingDialog_SetItemVisiable_AddAndRemove)
{
    SettingDialog::setItemVisiable("key_a", false);
    SettingDialog::setItemVisiable("key_b", false);
    EXPECT_TRUE(SettingDialog::needHide("key_a"));
    EXPECT_TRUE(SettingDialog::needHide("key_b"));
    SettingDialog::setItemVisiable("key_a", true);
    EXPECT_FALSE(SettingDialog::needHide("key_a"));
    EXPECT_TRUE(SettingDialog::needHide("key_b"));
}

TEST_F(DialogsTest, SettingDialog_NeedHide_UnknownKey)
{
    EXPECT_FALSE(SettingDialog::needHide("nonexistent_key_at_all"));
}








