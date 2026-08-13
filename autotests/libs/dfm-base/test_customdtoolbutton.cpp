// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customdtoolbutton.cpp
 * @brief Unit tests for CustomDToolButton (customdtoolbutton.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/dfmcustombuttons/customdtoolbutton.h>

#include <QApplication>
#include <QPaintEvent>

using namespace dfmbase;

TEST(CustomDToolButtonTest, ConstructDoesNotCrash)
{
    CustomDToolButton btn;
    (void)btn;
}

TEST(CustomDToolButtonTest, ConstructWithParentDoesNotCrash)
{
    QWidget parent;
    CustomDToolButton *btn = new CustomDToolButton(&parent);
    EXPECT_EQ(btn->parent(), &parent);
    delete btn;
}

TEST(CustomDToolButtonTest, SetTextDoesNotCrash)
{
    CustomDToolButton btn;
    btn.setText(QStringLiteral("test"));
    EXPECT_EQ(btn.text(), QStringLiteral("test"));
}

// ============================================================
// Additional coverage for CustomDToolButton
// ============================================================

TEST(CustomDToolButtonTest, PaintEventDoesNotCrash)
{
    CustomDToolButton btn;
    btn.setFixedSize(64, 64);
    QPaintEvent evt(QRect(0, 0, 64, 64));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&btn, &evt); });
}

TEST(CustomDToolButtonTest, PaintEventWhenDisabled)
{
    CustomDToolButton btn;
    btn.setEnabled(false);
    btn.setFixedSize(64, 64);
    QPaintEvent evt(QRect(0, 0, 64, 64));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&btn, &evt); });
}

TEST(CustomDToolButtonTest, InitStyleOptionEnabled)
{
    CustomDToolButton btn;
    QStyleOptionToolButton opt;
    EXPECT_NO_FATAL_FAILURE({ btn.initStyleOption(&opt); });
    // Color only set when isDown() or underMouse(), not in idle state
    // Just verify the call doesn't crash
    SUCCEED();
}

TEST(CustomDToolButtonTest, InitStyleOptionDisabled)
{
    CustomDToolButton btn;
    btn.setEnabled(false);
    QStyleOptionToolButton opt;
    EXPECT_NO_FATAL_FAILURE({ btn.initStyleOption(&opt); });
}
