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

// ============================================================
// Additional coverage for CheckBoxWithMessage
// ============================================================

TEST(CheckBoxWithMessageTest, SetCheckedTrueEmitsOnce)
{
    CheckBoxWithMessage w;
    QSignalSpy spy(&w, &CheckBoxWithMessage::stateChanged);
    w.setChecked(true);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), Qt::Checked);
}

TEST(CheckBoxWithMessageTest, ToggleChecked)
{
    CheckBoxWithMessage w;
    w.setChecked(true);
    w.setChecked(false);
    QSignalSpy spy(&w, &CheckBoxWithMessage::stateChanged);
    w.setChecked(true);
    EXPECT_EQ(spy.count(), 1);
}

TEST(CheckBoxWithMessageTest, SetDisplayTextTwice)
{
    CheckBoxWithMessage w;
    w.setDisplayText("first", "first message");
    w.setDisplayText("second", "second message");
    SUCCEED();
}

TEST(CheckBoxWithMessageTest, SetDisplayTextEmpty)
{
    CheckBoxWithMessage w;
    w.setDisplayText("", "");
    SUCCEED();
}
