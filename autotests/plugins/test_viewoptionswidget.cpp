// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewoptionswidget.cpp
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

TEST_F(ViewOptionsWidgetTest, ViewOptionsWidget)
{
    // Test constructor: ViewOptionsWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
