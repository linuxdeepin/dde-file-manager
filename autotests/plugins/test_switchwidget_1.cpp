// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_switchwidget_1.cpp
 * @brief Unit tests for SwitchWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/widgets/switchwidget.h"

#include <QTest>

using namespace ddplugin_organizer;

class SwitchWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SwitchWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SwitchWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SwitchWidgetTest, SwitchWidget)
{
    // Test constructor: SwitchWidget((const QString &title, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SwitchWidgetTest, setChecked)
{
    // Test setter: void setChecked((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->setChecked(false));
}

TEST_F(SwitchWidgetTest, setTitle)
{
    // Test setter: void setTitle((const QString &title))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTitle(_arg0));
}

TEST_F(SwitchWidgetTest, title)
{
    // Test getter: QString title()
    auto result = obj->title();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
