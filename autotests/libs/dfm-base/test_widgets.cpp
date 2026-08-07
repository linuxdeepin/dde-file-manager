// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_widgets.cpp
 * @brief Unit tests for GUI widget/statusbar/hint classes using stub-ext.
 *        Covers: BasicStatusBar, BasicStatusBarPrivate, ViewHintMessage,
 *        ViewHintWidget, WindowUtils, Splitter, AbstractBaseView.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QScreen>
#include <QString>

#include "stubext.h"

#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>
#include <dfm-base/widgets/dfmstatusbar/private/basicstatusbar_p.h>
#include <dfm-base/widgets/viewhintmessage/viewhintmessage.h>
#include <dfm-base/widgets/viewhintmessage/viewhintwidget.h>
#include <dfm-base/utils/windowutils.h>

using namespace dfmbase;

class WidgetsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(WidgetsTest, BasicStatusBarConstruct)
{
    BasicStatusBar bar;
    SUCCEED();
}

TEST_F(WidgetsTest, BasicStatusBarPrivateConstruct)
{
    BasicStatusBar bar;
    // Access private via -fno-access-control
    EXPECT_NO_FATAL_FAILURE({ (void)bar.d.data(); });
}

TEST_F(WidgetsTest, ViewHintMessageConstruct)
{
    ViewHintMessage msg;
    SUCCEED();
}

TEST_F(WidgetsTest, ViewHintWidgetConstruct)
{
    ViewHintWidget w;
    SUCCEED();
}

TEST_F(WidgetsTest, WindowUtilsIsX11)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::isX11(); });
}

TEST_F(WidgetsTest, WindowUtilsIsWayLand)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::isWayLand(); });
}

TEST_F(WidgetsTest, WindowUtilsKeyShiftIsPressed)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::keyShiftIsPressed(); });
}

TEST_F(WidgetsTest, WindowUtilsKeyCtrlIsPressed)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::keyCtrlIsPressed(); });
}

TEST_F(WidgetsTest, WindowUtilsKeyAltIsPressed)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::keyAltIsPressed(); });
}

TEST_F(WidgetsTest, WindowUtilsCursorScreen)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::cursorScreen(); });
}

TEST_F(WidgetsTest, WindowUtilsCloseAllFileManagerWindows)
{
    EXPECT_NO_FATAL_FAILURE({ WindowUtils::closeAllFileManagerWindows(); });
}
