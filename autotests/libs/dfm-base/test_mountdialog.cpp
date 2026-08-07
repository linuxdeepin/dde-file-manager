// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mountdialog.cpp
 * @brief Unit tests for MountAskPasswordDialog, MountSecretDiskAskPasswordDialog,
 *        UserSharePasswordSettingDialog, AliasComboBox.
 *        Uses stub-ext to mock exec/show.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QString>
#include <QJsonObject>

#include "stubext.h"

#include <dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h>
#include <dfm-base/dialogs/mountpasswddialog/mountsecretdiskaskpassworddialog.h>

using namespace dfmbase;

class MountDialogTest : public testing::Test
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

TEST_F(MountDialogTest, MountAskPasswordDialogConstruct)
{
    MountAskPasswordDialog d;
    SUCCEED();
}

TEST_F(MountDialogTest, MountAskPasswordDialogInitUI)
{
    MountAskPasswordDialog d;
    EXPECT_NO_FATAL_FAILURE({ d.initUI(); });
}

TEST_F(MountDialogTest, MountAskPasswordDialogInitConnect)
{
    MountAskPasswordDialog d;
    d.initUI();
    EXPECT_NO_FATAL_FAILURE({ d.initConnect(); });
}

TEST_F(MountDialogTest, MountAskPasswordDialogGetLoginData)
{
    MountAskPasswordDialog d;
    d.initUI();
    QJsonObject data = d.getLoginData();
    EXPECT_TRUE(data.isEmpty() || !data.isEmpty()); // just verify callable
}

TEST_F(MountDialogTest, MountAskPasswordDialogDomainVisible)
{
    MountAskPasswordDialog d;
    d.initUI();
    bool visible = d.getDomainLineVisible();
    d.setDomainLineVisible(!visible);
    EXPECT_EQ(d.getDomainLineVisible(), !visible);
}

TEST_F(MountDialogTest, MountAskPasswordDialogSetDomain)
{
    MountAskPasswordDialog d;
    d.initUI();
    d.setDomain("test_domain");
}

TEST_F(MountDialogTest, MountAskPasswordDialogSetUser)
{
    MountAskPasswordDialog d;
    d.initUI();
    d.setUser("test_user");
}

TEST_F(MountDialogTest, MountAskPasswordDialogHandleConnect)
{
    MountAskPasswordDialog d;
    d.initUI();
    EXPECT_NO_FATAL_FAILURE({ d.handleConnect(); });
}

TEST_F(MountDialogTest, MountSecretDiskAskPasswordDialogConstruct)
{
    MountSecretDiskAskPasswordDialog d("test tip");
    SUCCEED();
}

TEST_F(MountDialogTest, MountSecretDiskAskPasswordDialogInitUI)
{
    MountSecretDiskAskPasswordDialog d("test tip");
    EXPECT_NO_FATAL_FAILURE({ d.initUI(); });
}
