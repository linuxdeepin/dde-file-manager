// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_switchwidget.cpp
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

TEST_F(SwitchWidgetTest, checked)
{
    // Test bool getter: checked()
    bool result = obj->checked();
    EXPECT_FALSE(result);

}
