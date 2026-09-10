// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>

#include "stubext.h"

#include "views/resetpasswordview/vaultresetpasswordpages.h"
#include "utils/encryption/operatorcenter.h"
#include "dbus/vaultdbusutils.h"

DPVAULT_USE_NAMESPACE

class VaultResetPasswordPagesTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&OperatorCenter::resetPasswordByRecoveryKey,
                       [](OperatorCenter *, const QString &, const QString &, const QString &) -> bool { return true; });
        stub.set_lamda(&OperatorCenter::resetPasswordByOldPassword,
                       [](OperatorCenter *, const QString &, const QString &, const QString &) -> bool { return true; });
        stub.set_lamda(&VaultDBusUtils::restoreLeftoverErrorInputTimes, []() {});
        stub.set_lamda(&VaultDBusUtils::restoreNeedWaitMinutes, []() {});
        stub.set_lamda(&VaultDBusUtils::leftoverErrorInputTimesMinusOne, []() {});
        stub.set_lamda(&VaultDBusUtils::getLeftoverErrorInputTimes, []() -> int { return 3; });
        stub.set_lamda(&VaultDBusUtils::startTimerOfRestorePasswordInput, []() {});
        stub.set_lamda(&VaultDBusUtils::getNeedWaitMinutes, []() -> int { return 10; });
        pages = new VaultResetPasswordPages();
    }

    void TearDown() override
    {
        stub.clear();
        delete pages;
    }

    VaultResetPasswordPages *pages = nullptr;
};

TEST_F(VaultResetPasswordPagesTest, Constructor_CreatesView)
{
    EXPECT_NE(pages, nullptr);
}

TEST_F(VaultResetPasswordPagesTest, SwitchToOldPasswordView_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->switchToOldPasswordView());
}

TEST_F(VaultResetPasswordPagesTest, SwitchToKeyFileView_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->switchToKeyFileView());
}

TEST_F(VaultResetPasswordPagesTest, OnButtonClicked_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(pages->onButtonClicked(0, "test"));
}

TEST_F(VaultResetPasswordPagesTest, OnSetBtnEnabled_NoCrash)
{
    pages->switchToOldPasswordView();
    EXPECT_NO_FATAL_FAILURE(pages->onSetBtnEnabled(0, true));
}
