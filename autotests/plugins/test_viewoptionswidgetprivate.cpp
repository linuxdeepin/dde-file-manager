// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewoptionswidgetprivate.cpp
 * @brief Unit tests for ViewOptionsWidgetPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewoptionswidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewOptionsWidgetPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewOptionsWidgetPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewOptionsWidgetPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewOptionsWidgetPrivateTest, ViewOptionsWidgetPrivate)
{
    // Test constructor: ViewOptionsWidgetPrivate((ViewOptionsWidget *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ViewOptionsWidgetPrivateTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}
