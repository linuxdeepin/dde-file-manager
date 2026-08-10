// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customdtoolbutton.cpp
 * @brief Unit tests for CustomDToolButton (customdtoolbutton.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/dfmcustombuttons/customdtoolbutton.h>

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
