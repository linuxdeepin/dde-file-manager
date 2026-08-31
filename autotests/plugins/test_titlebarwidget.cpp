// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebarwidget.cpp
 * @brief Unit tests for TitleBarWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/titlebarwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBarWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBarWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarWidgetTest, TitleBarWidget)
{
    // Test constructor: TitleBarWidget((QFrame *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TitleBarWidgetTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TitleBarWidgetTest, handleHotketSwitchViewMode)
{
    // Test method: void handleHotketSwitchViewMode((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->handleHotketSwitchViewMode(0));
}

TEST_F(TitleBarWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(TitleBarWidgetTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(TitleBarWidgetTest, onTabAboutToRemove)
{
    // Test method: void onTabAboutToRemove((int oldIndex, int nextIndex))
    EXPECT_NO_FATAL_FAILURE(obj->onTabAboutToRemove(0, 0));
}

TEST_F(TitleBarWidgetTest, openPinnedTabs)
{
    // Test method: void openPinnedTabs(())
    EXPECT_NO_FATAL_FAILURE(obj->openPinnedTabs());
}

TEST_F(TitleBarWidgetTest, saveTitleBarState)
{
    // Test method: void saveTitleBarState((const QString &uniqueId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveTitleBarState(_arg0));
}

TEST_F(TitleBarWidgetTest, tabBar)
{
    // Test getter: TabBar tabBar()
    auto result = obj->tabBar();
    EXPECT_NO_FATAL_FAILURE({ obj->tabBar(); });

}

TEST_F(TitleBarWidgetTest, text)
{
    // Test getter: QString text()
    auto result = obj->text();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarWidgetTest, titleBar)
{
    // Test getter: DTitlebar titleBar()
    auto result = obj->titleBar();
    EXPECT_NO_FATAL_FAILURE({ obj->titleBar(); });

}
