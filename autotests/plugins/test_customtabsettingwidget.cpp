// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customtabsettingwidget.cpp
 * @brief Unit tests for CustomTabSettingWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/customtabsettingwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CustomTabSettingWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomTabSettingWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomTabSettingWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomTabSettingWidgetTest, CustomTabSettingWidget)
{
    // Test constructor: CustomTabSettingWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CustomTabSettingWidgetTest, clearCustomItems)
{
    // Test method: void clearCustomItems(())
    EXPECT_NO_FATAL_FAILURE(obj->clearCustomItems());
}

TEST_F(CustomTabSettingWidgetTest, removeRow)
{
    // Test method: bool removeRow((QWidget *w))
    auto result = obj->removeRow(nullptr);
    EXPECT_FALSE(result);

}
