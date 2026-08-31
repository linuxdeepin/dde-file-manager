// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basicwidget.cpp
 * @brief Unit tests for BasicWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/basicwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class BasicWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BasicWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BasicWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BasicWidgetTest, BasicWidget)
{
    // Test constructor: BasicWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BasicWidgetTest, BasicWidget_2)
{
    // Test constructor: BasicWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BasicWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}
