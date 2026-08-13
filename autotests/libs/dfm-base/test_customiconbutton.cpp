// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customiconbutton.cpp
 * @brief Unit tests for CustomDIconButton (customiconbutton.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/dfmcustombuttons/customiconbutton.h>

#include <DStyle>
#include <QApplication>
#include <QPaintEvent>

using namespace dfmbase;

TEST(CustomDIconButtonTest, ConstructWithParentDoesNotCrash)
{
    CustomDIconButton btn;
    EXPECT_TRUE(btn.isFlat());
}

TEST(CustomDIconButtonTest, ConstructWithIconTypeDoesNotCrash)
{
    CustomDIconButton btn(DTK_NAMESPACE::Widget::DStyle::SP_DeleteButton);
    EXPECT_TRUE(btn.isFlat());
}

TEST(CustomDIconButtonTest, ConstructWithExplicitParentDoesNotCrash)
{
    QWidget parent;
    CustomDIconButton *btn = new CustomDIconButton(&parent);
    EXPECT_EQ(btn->parent(), &parent);
    delete btn;
}

// ============================================================
// Additional coverage for CustomDIconButton
// ============================================================

TEST(CustomDIconButtonTest, PaintEventDoesNotCrash)
{
    CustomDIconButton btn;
    btn.setFixedSize(32, 32);
    QPaintEvent evt(QRect(0, 0, 32, 32));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&btn, &evt); });
}

TEST(CustomDIconButtonTest, PaintEventWhenDisabled)
{
    CustomDIconButton btn(DTK_NAMESPACE::Widget::DStyle::SP_DeleteButton);
    btn.setEnabled(false);
    btn.setFixedSize(32, 32);
    QPaintEvent evt(QRect(0, 0, 32, 32));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&btn, &evt); });
}

TEST(CustomDIconButtonTest, PaintEventWhenChecked)
{
    CustomDIconButton btn(DTK_NAMESPACE::Widget::DStyle::SP_DeleteButton);
    btn.setCheckable(true);
    btn.setChecked(true);
    btn.setFixedSize(32, 32);
    QPaintEvent evt(QRect(0, 0, 32, 32));
    EXPECT_NO_FATAL_FAILURE({ QApplication::sendEvent(&btn, &evt); });
}
