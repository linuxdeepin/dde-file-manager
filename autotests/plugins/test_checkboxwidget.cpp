// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_checkboxwidget.cpp
 * @brief Unit tests for CheckBoxWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/widgets/checkboxwidget.h"

#include <QTest>

using namespace ddplugin_organizer;

class CheckBoxWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CheckBoxWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CheckBoxWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CheckBoxWidgetTest, CheckBoxWidget)
{
    // Test constructor: CheckBoxWidget((const QString &text, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CheckBoxWidgetTest, checked)
{
    // Test bool getter: checked()
    bool result = obj->checked();
    EXPECT_FALSE(result);

}

TEST_F(CheckBoxWidgetTest, setChecked)
{
    // Test setter: void setChecked((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->setChecked(false));
}
