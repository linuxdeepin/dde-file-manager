// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searcheditwidget.cpp
 * @brief Unit tests for SearchEditWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/searcheditwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class SearchEditWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchEditWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchEditWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchEditWidgetTest, SearchEditWidget)
{
    // Test constructor: SearchEditWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchEditWidgetTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SearchEditWidgetTest, text)
{
    // Test getter: QString text()
    auto result = obj->text();
    EXPECT_TRUE(result.isEmpty());

}
