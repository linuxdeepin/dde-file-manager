// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewoptionswidget_1.cpp
 * @brief Unit tests for ViewOptionsWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewoptionswidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewOptionsWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewOptionsWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewOptionsWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewOptionsWidgetTest, hideEvent)
{
    // Test event handler: hideEvent((QHideEvent *event))
    QHideEvent _event(QHideEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->hideEvent(&_event));
}
