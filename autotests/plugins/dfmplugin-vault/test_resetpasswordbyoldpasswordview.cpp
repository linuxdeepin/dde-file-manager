// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QSignalSpy>

#include "stubext.h"

#define private public
#define protected public
#include "views/resetpasswordview/resetpasswordbyoldpasswordview.h"
#undef protected
#undef private
#include "utils/encryption/operatorcenter.h"
#include "dbus/vaultdbusutils.h"

DPVAULT_USE_NAMESPACE

class ResetPasswordByOldPasswordViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&OperatorCenter::resetPasswordByOldPassword,
                       [](OperatorCenter *, const QString &, const QString &, const QString &) -> bool { return true; });
        stub.set_lamda(&VaultDBusUtils::restoreLeftoverErrorInputTimes, []() {});
        stub.set_lamda(&VaultDBusUtils::restoreNeedWaitMinutes, []() {});
        stub.set_lamda(&VaultDBusUtils::leftoverErrorInputTimesMinusOne, []() {});
        stub.set_lamda(&VaultDBusUtils::getLeftoverErrorInputTimes, []() -> int { return 3; });
        stub.set_lamda(&VaultDBusUtils::startTimerOfRestorePasswordInput, []() {});
        stub.set_lamda(&VaultDBusUtils::getNeedWaitMinutes, []() -> int { return 10; });
        view = new ResetPasswordByOldPasswordView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    ResetPasswordByOldPasswordView *view = nullptr;
};

TEST_F(ResetPasswordByOldPasswordViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(ResetPasswordByOldPasswordViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

TEST_F(ResetPasswordByOldPasswordViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(ResetPasswordByOldPasswordViewTest, OnPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onPasswordChanged());
}

TEST_F(ResetPasswordByOldPasswordViewTest, OnOldPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onOldPasswordChanged("oldpass"));
}

TEST_F(ResetPasswordByOldPasswordViewTest, OnNewPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onNewPasswordChanged("TestPass1@"));
}

TEST_F(ResetPasswordByOldPasswordViewTest, OnRepeatPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onRepeatPasswordChanged("TestPass1@"));
}

TEST_F(ResetPasswordByOldPasswordViewTest, ButtonClicked_IndexZero_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(0, "Cancel"));
}

TEST_F(ResetPasswordByOldPasswordViewTest, ButtonClicked_IndexOne_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(1, "Confirm"));
}

TEST_F(ResetPasswordByOldPasswordViewTest, ButtonClicked_InvalidIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(99, "Unknown"));
}

// --- checkRepeatPassword ---

TEST_F(ResetPasswordByOldPasswordViewTest, CheckRepeatPassword_Match_ReturnsTrue)
{
    view->newPasswordEdit->setText("TestPass1@");
    view->repeatPasswordEdit->setText("TestPass1@");
    EXPECT_TRUE(view->checkRepeatPassword());
}

TEST_F(ResetPasswordByOldPasswordViewTest, CheckRepeatPassword_NoMatch_ReturnsFalse)
{
    view->newPasswordEdit->setText("TestPass1@");
    view->repeatPasswordEdit->setText("Different1@");
    EXPECT_FALSE(view->checkRepeatPassword());
}
