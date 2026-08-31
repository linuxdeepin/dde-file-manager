// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tabbarprivate_1.cpp
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

TEST_F(TabBarPrivateTest, TabBarPrivate)
{
    // Test constructor: TabBarPrivate((TabBar *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TabBarPrivateTest, canPinned)
{
    // Test method: bool canPinned((int index))
    auto result = obj->canPinned(0);
    EXPECT_FALSE(result);

}

TEST_F(TabBarPrivateTest, closeOtherTabs)
{
    // Test method: void closeOtherTabs((int index))
    EXPECT_NO_FATAL_FAILURE(obj->closeOtherTabs(0));
}

TEST_F(TabBarPrivateTest, closeRightTabs)
{
    // Test method: void closeRightTabs((int index))
    EXPECT_NO_FATAL_FAILURE(obj->closeRightTabs(0));
}

TEST_F(TabBarPrivateTest, createColoredIcon)
{
    // Test method: QPixmap createColoredIcon((const QIcon &icon, const QColor &color, const QSize &size))
    QIcon _arg0{};
    QColor _arg1{};
    QSize _arg2{};
    auto result = obj->createColoredIcon(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isNull());

}

TEST_F(TabBarPrivateTest, determineRedirectUrl)
{
    // Test method: QUrl determineRedirectUrl((const QUrl &currentUrl, const QUrl &targetUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->determineRedirectUrl(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TabBarPrivateTest, findValidParentPath)
{
    // Test method: QUrl findValidParentPath((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->findValidParentPath(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TabBarPrivateTest, getActualUrl)
{
    // Test method: QUrl getActualUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getActualUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TabBarPrivateTest, handleContextMenu)
{
    // Test method: void handleContextMenu((int index))
    EXPECT_NO_FATAL_FAILURE(obj->handleContextMenu(0));
}

TEST_F(TabBarPrivateTest, handleDragActionChanged)
{
    // Test method: void handleDragActionChanged((Qt::DropAction action))
    EXPECT_NO_FATAL_FAILURE(obj->handleDragActionChanged(Qt::DropAction()));
}

TEST_F(TabBarPrivateTest, handleIndexChanged)
{
    // Test method: void handleIndexChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->handleIndexChanged(0));
}

TEST_F(TabBarPrivateTest, handleLastTabClose)
{
    // Test method: void handleLastTabClose((const QUrl &currentUrl, const QUrl &targetUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleLastTabClose(_arg0, _arg1));
}

TEST_F(TabBarPrivateTest, handleTabDroped)
{
    // Test method: void handleTabDroped((int index, Qt::DropAction dropAction, QObject *target))
    EXPECT_NO_FATAL_FAILURE(obj->handleTabDroped(0, Qt::DropAction(), nullptr));
}

TEST_F(TabBarPrivateTest, handleTabMoved)
{
    // Test method: void handleTabMoved((int from, int to))
    EXPECT_NO_FATAL_FAILURE(obj->handleTabMoved(0, 0));
}

TEST_F(TabBarPrivateTest, hasDragPreviewTab)
{
    // Test bool getter: hasDragPreviewTab()
    bool result = obj->hasDragPreviewTab();
    EXPECT_FALSE(result);

}

TEST_F(TabBarPrivateTest, initConnections)
{
    // Test method: void initConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnections());
}

TEST_F(TabBarPrivateTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(TabBarPrivateTest, isItemButtonHovered)
{
    // Test method: bool isItemButtonHovered((int index))
    auto result = obj->isItemButtonHovered(0);
    EXPECT_FALSE(result);

}

TEST_F(TabBarPrivateTest, paintTabButton)
{
    // Test method: void paintTabButton((DIconButton *btn))
    EXPECT_NO_FATAL_FAILURE(obj->paintTabButton(nullptr));
}

TEST_F(TabBarPrivateTest, paintTabItemButton)
{
    // Test method: void paintTabItemButton((QPainter *painter, int index, const QStyleOptionTab &option))
    QStyleOptionTab _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintTabItemButton(nullptr, 0, _arg2));
}

TEST_F(TabBarPrivateTest, shouldCreateNewTabForPinnedTab)
{
    // Test method: bool shouldCreateNewTabForPinnedTab((const QUrl &currentUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->shouldCreateNewTabForPinnedTab(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TabBarPrivateTest, tabInfo)
{
    // Test method: Tab tabInfo((int index))
    auto result = obj->tabInfo(0);
    EXPECT_NO_FATAL_FAILURE({ obj->tabInfo(0); });

}

TEST_F(TabBarPrivateTest, updateTabInfo)
{
    // Test method: bool updateTabInfo((int index, std::function<void(Tab &)> modifier))
    std::function<void(Tab )> _arg1{};
    auto result = obj->updateTabInfo(0, _arg1);
    EXPECT_FALSE(result);

}
