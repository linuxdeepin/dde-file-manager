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
