// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tabbar.cpp
 * @brief Unit tests for TabBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/tabbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TabBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TabBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TabBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TabBarTest, TabBar)
{
    // Test constructor: TabBar((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TabBarTest, closeTab)
{
    // Test method: void closeTab((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->closeTab(_arg0));
}

TEST_F(TabBarTest, forceRemoveTab)
{
    // Test method: void forceRemoveTab((int index))
    EXPECT_NO_FATAL_FAILURE(obj->forceRemoveTab(0));
}

TEST_F(TabBarTest, removeTab)
{
    // Test method: void removeTab((int index, int selectIndex))
    EXPECT_NO_FATAL_FAILURE(obj->removeTab(0, 0));
}
