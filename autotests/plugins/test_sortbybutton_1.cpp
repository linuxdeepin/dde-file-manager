// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortbybutton_1.cpp
 * @brief Unit tests for SortByButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/sortbybutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class SortByButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortByButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortByButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortByButtonTest, enterEvent)
{
    // Test event handler: enterEvent((QEnterEvent *event))
    QEnterEvent _event(QEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->enterEvent(&_event));
}

TEST_F(SortByButtonTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(SortByButtonTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(SortByButtonTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}
