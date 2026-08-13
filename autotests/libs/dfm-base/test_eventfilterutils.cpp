// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventfilterutils.cpp
 * @brief Unit tests for EventFilter::handleRightClickOutsideMenu
 *        (eventfilterutils.cpp)
 *
 * handleRightClickOutsideMenu early-returns false unless the current Qt
 * platform is X11. Under the offscreen platform it must return false for
 * any event, covering the non-X11 guard branch without real X11 hardware.
 */

#include <gtest/gtest.h>
#include <dfm-base/utils/eventfilterutils.h>
#include <dfm-base/utils/windowutils.h>
#include "stubext.h"

#include <QMouseEvent>
#include <QEvent>
#include <QGuiApplication>
#include <QPointF>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QMenu>

using namespace dfmbase;
using namespace dfmbase::EventFilter;

TEST(EventFilterUtilsTest, ReturnsFalseForNonX11PlatformMouseButtonPress)
{
    // Under X11 the function proceeds past the platform guard; under offscreen
    // it returns false immediately.  We verify the guard branch is correct for
    // the *current* platform rather than asserting a specific platform.
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0),
                      Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    if (!WindowUtils::isX11()) {
        EXPECT_FALSE(handleRightClickOutsideMenu(&press));
    } else {
        // On X11 the function continues past the guard; with no active popup
        // menu it should still return false.
        EXPECT_FALSE(handleRightClickOutsideMenu(&press));
    }
}

TEST(EventFilterUtilsTest, ReturnsFalseForNonMouseButtonEvent)
{
    // Even if isX11 were true, a non-MouseButtonPress event returns false.
    // Under offscreen isX11() is false so this returns false immediately.
    QEvent other(QEvent::Wheel);
    EXPECT_FALSE(handleRightClickOutsideMenu(&other));
}

TEST(EventFilterUtilsTest, ReturnsFalseForNullEvent)
{
    // Passing a null event pointer must not crash; under offscreen the X11
    // guard returns false before dereferencing.
    EXPECT_FALSE(handleRightClickOutsideMenu(nullptr));
}

TEST(EventFilterUtilsTest, ReturnsFalseForLeftButtonClickUnderNonX11)
{
    QMouseEvent left(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0),
                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&left));
}

// ============================================================
// Additional coverage for EventFilterUtils
// ============================================================

TEST(EventFilterUtilsTest, ReturnsFalseForMiddleButtonClick)
{
    QMouseEvent middle(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0),
                       Qt::MiddleButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&middle));
}

TEST(EventFilterUtilsTest, ReturnsFalseForMouseRelease)
{
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 0),
                        Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&release));
}

TEST(EventFilterUtilsTest, ReturnsFalseForMouseButtonDblClick)
{
    QMouseEvent dbl(QEvent::MouseButtonDblClick, QPointF(0, 0), QPointF(0, 0),
                    Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&dbl));
}

TEST(EventFilterUtilsTest, ReturnsFalseForMouseMove)
{
    QMouseEvent move(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10),
                     Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&move));
}

TEST(EventFilterUtilsTest, ReturnsFalseForKeyPressEvent)
{
    QKeyEvent key(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&key));
}

TEST(EventFilterUtilsTest, ReturnsFalseForTimerEvent)
{
    QTimerEvent timer(0);
    EXPECT_FALSE(handleRightClickOutsideMenu(&timer));
}

TEST(EventFilterUtilsTest, ReturnsFalseForCloseEvent)
{
    // Not a mouse button press
    QEvent close(QEvent::Close);
    EXPECT_FALSE(handleRightClickOutsideMenu(&close));
}

TEST(EventFilterUtilsTest, RightClickWithNoActivePopupReturnsFalse)
{
    // Even on X11, if no active popup menu exists, should return false
    QMouseEvent rightClick(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100),
                           Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    // No menu shown, so should return false
    bool result = handleRightClickOutsideMenu(&rightClick);
    // Result is false either because not X11 or no active popup
    EXPECT_FALSE(result);
}

TEST(EventFilterUtilsTest, WithContextNonNull)
{
    QObject ctx;
    QMouseEvent rightClick(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0),
                           Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&rightClick, &ctx));
}

TEST(EventFilterUtilsTest, WithRightButtonAndExtraButtons)
{
    QMouseEvent rightClick(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0),
                           Qt::RightButton, Qt::LeftButton | Qt::RightButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&rightClick));
}

TEST(EventFilterUtilsTest, RightClickOutsideMenuStubbedX11)
{
    // Stub WindowUtils::isX11 to return true so the function proceeds past the guard
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::isX11, []() -> bool { return true; });

    // No active popup menu → should return false
    QMouseEvent rightClick(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100),
                           Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    bool result = handleRightClickOutsideMenu(&rightClick);
    EXPECT_FALSE(result);
}

TEST(EventFilterUtilsTest, RightClickOutsideMenuWithMenuShownStubbedX11)
{
    // Stub isX11 to true
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::isX11, []() -> bool { return true; });

    // Create a QMenu and show it (popup)
    QMenu menu;
    menu.addAction("Test Item 1");
    menu.addAction("Test Item 2");
    menu.show();
    QCoreApplication::processEvents();

    // Click outside the menu
    QMouseEvent rightClick(QEvent::MouseButtonPress, QPointF(5000, 5000), QPointF(5000, 5000),
                           Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    bool result = handleRightClickOutsideMenu(&rightClick);
    EXPECT_TRUE(result);

    menu.close();
}

TEST(EventFilterUtilsTest, RightClickInsideMenuStubbedX11)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::isX11, []() -> bool { return true; });

    QMenu menu;
    menu.addAction("Item");
    menu.show();
    QCoreApplication::processEvents();

    // Click inside the menu geometry
    QPoint insidePos = menu.geometry().center();
    QMouseEvent rightClick(QEvent::MouseButtonPress, insidePos, insidePos,
                           Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    bool result = handleRightClickOutsideMenu(&rightClick);
    EXPECT_FALSE(result);

    menu.close();
}

TEST(EventFilterUtilsTest, RightClickOnWidgetWithContextMenuStubbedX11)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::isX11, []() -> bool { return true; });

    // Create a menu and show it
    QMenu menu;
    menu.show();
    QCoreApplication::processEvents();

    // Click far outside menu
    QMouseEvent rightClick(QEvent::MouseButtonPress, QPointF(-100, -100), QPointF(-100, -100),
                           Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    bool result = handleRightClickOutsideMenu(&rightClick);
    // True because click is outside menu and close() is called
    EXPECT_TRUE(result);

    menu.close();
}
