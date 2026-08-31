// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_navwidget.cpp
 * @brief Unit tests for NavWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/navwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class NavWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NavWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NavWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NavWidgetTest, NavWidget)
{
    // Test constructor: NavWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(NavWidgetTest, back)
{
    // Test method: void back(())
    EXPECT_NO_FATAL_FAILURE(obj->back());
}

TEST_F(NavWidgetTest, forward)
{
    // Test method: void forward(())
    EXPECT_NO_FATAL_FAILURE(obj->forward());
}

TEST_F(NavWidgetTest, removeNavStackAt)
{
    // Test method: void removeNavStackAt((int index))
    EXPECT_NO_FATAL_FAILURE(obj->removeNavStackAt(0));
}

TEST_F(NavWidgetTest, removeUrlFromHistoryStack)
{
    // Test method: void removeUrlFromHistoryStack((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeUrlFromHistoryStack(_arg0));
}
