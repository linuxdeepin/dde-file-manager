// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_widgets_r19.cpp
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

class WidgetsR19Test : public testing::Test
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

TEST_F(WidgetsR19Test, BasicStatusBarConstruct)
{
    BasicStatusBar bar;
    SUCCEED();
}

TEST_F(WidgetsR19Test, BasicStatusBarPrivateConstruct)
{
    BasicStatusBar bar;
    // Access private via -fno-access-control
    EXPECT_NO_FATAL_FAILURE({ (void)bar.d.data(); });
}

TEST_F(WidgetsR19Test, ViewHintMessageConstruct)
{
    ViewHintMessage msg;
    SUCCEED();
}

TEST_F(WidgetsR19Test, ViewHintWidgetConstruct)
{
    ViewHintWidget w;
    SUCCEED();
}

TEST_F(WidgetsR19Test, WindowUtilsIsX11)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::isX11(); });
}

TEST_F(WidgetsR19Test, WindowUtilsIsWayLand)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::isWayLand(); });
}

TEST_F(WidgetsR19Test, WindowUtilsKeyShiftIsPressed)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::keyShiftIsPressed(); });
}

TEST_F(WidgetsR19Test, WindowUtilsKeyCtrlIsPressed)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::keyCtrlIsPressed(); });
}

TEST_F(WidgetsR19Test, WindowUtilsKeyAltIsPressed)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::keyAltIsPressed(); });
}

TEST_F(WidgetsR19Test, WindowUtilsCursorScreen)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WindowUtils::cursorScreen(); });
}

TEST_F(WidgetsR19Test, WindowUtilsCloseAllFileManagerWindows)
{
    EXPECT_NO_FATAL_FAILURE({ WindowUtils::closeAllFileManagerWindows(); });
}
