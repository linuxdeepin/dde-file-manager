// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QShowEvent>

#include "stubext.h"

#define private public
#define protected public
#include "views/resetpasswordview/resetpasswordbykeyfileview.h"
#undef protected
#undef private
#include "utils/encryption/operatorcenter.h"
#include "dbus/vaultdbusutils.h"

DPVAULT_USE_NAMESPACE

class ResetPasswordByKeyFileViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&OperatorCenter::resetPasswordByRecoveryKey,
                       [](OperatorCenter *, const QString &, const QString &, const QString &) -> bool { return true; });
        stub.set_lamda(&VaultDBusUtils::restoreLeftoverErrorInputTimes, []() {});
        stub.set_lamda(&VaultDBusUtils::restoreNeedWaitMinutes, []() {});
        stub.set_lamda(&VaultDBusUtils::leftoverErrorInputTimesMinusOne, []() {});
        stub.set_lamda(&VaultDBusUtils::getLeftoverErrorInputTimes, []() -> int { return 3; });
        stub.set_lamda(&VaultDBusUtils::startTimerOfRestorePasswordInput, []() {});
        stub.set_lamda(&VaultDBusUtils::getNeedWaitMinutes, []() -> int { return 10; });
        view = new ResetPasswordByKeyFileView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    ResetPasswordByKeyFileView *view = nullptr;
};

TEST_F(ResetPasswordByKeyFileViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(ResetPasswordByKeyFileViewTest, BtnText_ReturnsTwoButtons)
{
    QStringList btns = view->btnText();
    EXPECT_EQ(btns.size(), 2);
}

TEST_F(ResetPasswordByKeyFileViewTest, TitleText_ReturnsNonEmpty)
{
    QString title = view->titleText();
    EXPECT_FALSE(title.isEmpty());
}

TEST_F(ResetPasswordByKeyFileViewTest, OnPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onPasswordChanged());
}

TEST_F(ResetPasswordByKeyFileViewTest, OnKeyFileSelected_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onKeyFileSelected("/tmp/testkey.key"));
}

TEST_F(ResetPasswordByKeyFileViewTest, OnNewPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onNewPasswordChanged("TestPass1@"));
}

TEST_F(ResetPasswordByKeyFileViewTest, OnRepeatPasswordChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onRepeatPasswordChanged("TestPass1@"));
}

TEST_F(ResetPasswordByKeyFileViewTest, ButtonClicked_IndexZero_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(0, "Cancel"));
}

TEST_F(ResetPasswordByKeyFileViewTest, ButtonClicked_IndexOne_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(1, "Confirm"));
}

TEST_F(ResetPasswordByKeyFileViewTest, ButtonClicked_InvalidIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->buttonClicked(99, "Unknown"));
}

// --- checkPassword ---

TEST_F(ResetPasswordByKeyFileViewTest, CheckPassword_Valid_ReturnsTrue)
{
    EXPECT_TRUE(view->checkPassword("Valid1!a"));
}

TEST_F(ResetPasswordByKeyFileViewTest, CheckPassword_TooShort_ReturnsFalse)
{
    EXPECT_FALSE(view->checkPassword("Ab1"));
}

TEST_F(ResetPasswordByKeyFileViewTest, CheckPassword_TooLong_ReturnsFalse)
{
    QString longPwd(200, 'a');
    EXPECT_FALSE(view->checkPassword(longPwd));
}

// --- checkRepeatPassword ---

TEST_F(ResetPasswordByKeyFileViewTest, CheckRepeatPassword_Match_ReturnsTrue)
{
    view->newPasswordEdit->setText("TestPass1@");
    view->repeatPasswordEdit->setText("TestPass1@");
    EXPECT_TRUE(view->checkRepeatPassword());
}

TEST_F(ResetPasswordByKeyFileViewTest, CheckRepeatPassword_NoMatch_ReturnsFalse)
{
    view->newPasswordEdit->setText("TestPass1@");
    view->repeatPasswordEdit->setText("Different1@");
    EXPECT_FALSE(view->checkRepeatPassword());
}

// --- showEvent ---

TEST_F(ResetPasswordByKeyFileViewTest, ShowEvent_NoCrash)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(view->showEvent(&event));
}
