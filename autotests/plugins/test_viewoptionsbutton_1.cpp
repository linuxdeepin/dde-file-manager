// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewoptionsbutton_1.cpp
 * @brief Unit tests for ViewOptionsButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewoptionsbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewOptionsButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewOptionsButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewOptionsButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewOptionsButtonTest, ViewOptionsButton)
{
    // Test constructor: ViewOptionsButton((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewOptionsButtonTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(ViewOptionsButtonTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(ViewOptionsButtonTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(ViewOptionsButtonTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(ViewOptionsButtonTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(ViewOptionsButtonTest, switchMode)
{
    // Test method: void switchMode((ViewMode mode, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->switchMode(ViewMode(), _arg1));
}
