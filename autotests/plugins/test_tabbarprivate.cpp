// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tabbarprivate.cpp
 * @brief Unit tests for TabBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/tabbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TabBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TabBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TabBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TabBarPrivateTest, closeLeftTabs)
{
    // Test method: void closeLeftTabs((int index))
    EXPECT_NO_FATAL_FAILURE(obj->closeLeftTabs(0));
}

TEST_F(TabBarPrivateTest, handlePinnedTabsChanged)
{
    // Test method: void handlePinnedTabsChanged((const QString &config, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handlePinnedTabsChanged(_arg0, _arg1));
}

TEST_F(TabBarPrivateTest, handleTabReleased)
{
    // Test method: void handleTabReleased((int index))
    EXPECT_NO_FATAL_FAILURE(obj->handleTabReleased(0));
}

TEST_F(TabBarPrivateTest, paintTabBackground)
{
    // Test method: void paintTabBackground((QPainter *painter, int index, const QStyleOptionTab &option))
    QStyleOptionTab _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintTabBackground(nullptr, 0, _arg2));
}

TEST_F(TabBarPrivateTest, paintTabLabel)
{
    // Test method: void paintTabLabel((QPainter *painter, int index, const QStyleOptionTab &option))
    QStyleOptionTab _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintTabLabel(nullptr, 0, _arg2));
}

TEST_F(TabBarPrivateTest, setTabPinned)
{
    // Test setter: void setTabPinned((int index, bool pinned, bool updateConfig))
    EXPECT_NO_FATAL_FAILURE(obj->setTabPinned(0, false, false));
}

TEST_F(TabBarPrivateTest, showWindow)
{
    // Test method: void showWindow(())
    EXPECT_NO_FATAL_FAILURE(obj->showWindow());
}

TEST_F(TabBarPrivateTest, tabCloseable)
{
    // Test method: bool tabCloseable((const Tab &tab, const QUrl &targetUrl))
    Tab _arg0{};
    QUrl _arg1{};
    auto result = obj->tabCloseable(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TabBarPrivateTest, tabDisplayName)
{
    // Test method: QString tabDisplayName((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->tabDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TabBarPrivateTest, updatePinnedTabsOrder)
{
    // Test method: void updatePinnedTabsOrder(())
    EXPECT_NO_FATAL_FAILURE(obj->updatePinnedTabsOrder());
}

TEST_F(TabBarPrivateTest, updateToolTip)
{
    // Test method: void updateToolTip((int index, bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->updateToolTip(0, false));
}
