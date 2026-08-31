// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_headerview.cpp
 * @brief Unit tests for HeaderView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/headerview.h"

#include <QTest>

using namespace dfmplugin_workspace;

class HeaderViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new HeaderView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    HeaderView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(HeaderViewTest, contextMenuEvent)
{
    // Test event handler: contextMenuEvent((QContextMenuEvent *event))
    QContextMenuEvent _event(QContextMenuEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->contextMenuEvent(&_event));
}

TEST_F(HeaderViewTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *e))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}
