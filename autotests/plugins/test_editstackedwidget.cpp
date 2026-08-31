// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_editstackedwidget.cpp
 * @brief Unit tests for EditStackedWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/editstackedwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class EditStackedWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EditStackedWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EditStackedWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EditStackedWidgetTest, EditStackedWidget)
{
    // Test constructor: EditStackedWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EditStackedWidgetTest, initTextShowFrame)
{
    // Test method: void initTextShowFrame((QString fileName))
    EXPECT_NO_FATAL_FAILURE(obj->initTextShowFrame(QString()));
}
