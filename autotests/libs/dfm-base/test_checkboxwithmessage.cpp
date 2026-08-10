// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_checkboxwithmessage.cpp
 * @brief Unit tests for CheckBoxWithMessage (checkboxwithmessage.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/dialogs/settingsdialog/controls/checkboxwithmessage.h>

#include <QSignalSpy>
#include <QString>

TEST(CheckBoxWithMessageTest, ConstructDoesNotCrash)
{
    CheckBoxWithMessage w;
    (void)w;
}

TEST(CheckBoxWithMessageTest, SetDisplayTextDoesNotCrash)
{
    CheckBoxWithMessage w;
    w.setDisplayText(QStringLiteral("check me"), QStringLiteral("message text"));
}

TEST(CheckBoxWithMessageTest, SetCheckedTrueEmitsStateChanged)
{
    CheckBoxWithMessage w;
    QSignalSpy spy(&w, &CheckBoxWithMessage::stateChanged);
    w.setChecked(true);
    EXPECT_GE(spy.count(), 1);
}

TEST(CheckBoxWithMessageTest, SetCheckedFalseDoesNotEmitWhenAlreadyUnchecked)
{
    CheckBoxWithMessage w;
    QSignalSpy spy(&w, &CheckBoxWithMessage::stateChanged);
    w.setChecked(false);   // already unchecked
    EXPECT_EQ(spy.count(), 0);
}
