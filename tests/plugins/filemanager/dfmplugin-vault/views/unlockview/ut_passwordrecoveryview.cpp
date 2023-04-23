// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/unlockview/passwordrecoveryview.h"
#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_PasswordRecoveryView, btnText)
{
    PasswordRecoveryView view;
    EXPECT_FALSE(view.btnText().isEmpty());
}

TEST(UT_PasswordRecoveryView, titleText)
{
    PasswordRecoveryView view;
    EXPECT_FALSE(view.titleText().isEmpty());
}

TEST(UT_PasswordRecoveryView, setResultsPage)
{
    PasswordRecoveryView view;
    view.setResultsPage("123456");
    EXPECT_FALSE(view.passwordMsg->text().isEmpty());
}

TEST(UT_PasswordRecoveryView, buttonClicked_Zero)
{
    bool isJump { false };
    PasswordRecoveryView view;
    QObject::connect(&view, &PasswordRecoveryView::signalJump, [ &isJump ] { isJump = true; });
    view.buttonClicked(0, "");
    EXPECT_TRUE(isJump);
}

TEST(UT_PasswordRecoveryView, buttonClicked_One)
{
    bool isClosed { false };
    PasswordRecoveryView view;
    QObject::connect(&view, &PasswordRecoveryView::sigCloseDialog, [ &isClosed ] { isClosed = true; });
    view.buttonClicked(1, "");
    EXPECT_TRUE(isClosed);
}
