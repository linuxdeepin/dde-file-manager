// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basicstatusbar.cpp
 * @brief Unit tests for BasicStatusBar (basicstatusbar.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>

#include <QSize>
#include <QString>

using namespace dfmbase;

TEST(BasicStatusBarTest, ConstructDoesNotCrash)
{
    BasicStatusBar bar;
    (void)bar;
}

TEST(BasicStatusBarTest, SizeHintHeightAtLeast32)
{
    BasicStatusBar bar;
    QSize hint = bar.sizeHint();
    EXPECT_GE(hint.height(), 32);
}

TEST(BasicStatusBarTest, ClearLayoutAndAnchorsDoesNotCrash)
{
    BasicStatusBar bar;
    bar.clearLayoutAndAnchors();
    SUCCEED();
}

TEST(BasicStatusBarTest, ItemSelectedWithNoItemsDoesNotCrash)
{
    BasicStatusBar bar;
    bar.itemSelected(0, 0, 0, {});
    SUCCEED();
}

TEST(BasicStatusBarTest, SetTipTextDoesNotCrash)
{
    BasicStatusBar bar;
    bar.setTipText(QStringLiteral("test tip"));
    SUCCEED();
}
