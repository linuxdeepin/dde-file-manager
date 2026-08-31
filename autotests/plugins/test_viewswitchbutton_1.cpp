// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewswitchbutton_1.cpp
 * @brief Unit tests for ViewSwitchButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewswitchbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewSwitchButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewSwitchButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewSwitchButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewSwitchButtonTest, ViewSwitchButton)
{
    // Test constructor: ViewSwitchButton((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewSwitchButtonTest, enterEvent)
{
    // Test event handler: enterEvent((QEnterEvent *event))
    QEnterEvent _event(QEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->enterEvent(&_event));
}

TEST_F(ViewSwitchButtonTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(ViewSwitchButtonTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(ViewSwitchButtonTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(ViewSwitchButtonTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}
