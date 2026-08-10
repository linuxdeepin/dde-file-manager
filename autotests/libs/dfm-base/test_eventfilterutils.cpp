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

#include <QMouseEvent>
#include <QEvent>
#include <QGuiApplication>
#include <QPointF>

using namespace dfmbase;
using namespace dfmbase::EventFilter;

TEST(EventFilterUtilsTest, ReturnsFalseForNonX11PlatformMouseButtonPress)
{
    ASSERT_FALSE(WindowUtils::isX11());   // offscreen guard
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0),
                      Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handleRightClickOutsideMenu(&press));
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
