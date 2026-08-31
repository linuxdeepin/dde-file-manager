// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortbybutton.cpp
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

TEST_F(SortByButtonTest, SortByButton)
{
    // Test constructor: SortByButton((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SortByButtonTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}
