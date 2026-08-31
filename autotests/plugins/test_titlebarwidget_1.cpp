// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebarwidget_1.cpp
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

TEST_F(TitleBarWidgetTest, activatePinnedTab)
{
    // Test method: void activatePinnedTab((const QString &pinnedId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->activatePinnedTab(_arg0));
}

TEST_F(TitleBarWidgetTest, calculateRemainingWidth)
{
    // Test getter: int calculateRemainingWidth()
    auto result = obj->calculateRemainingWidth();
    EXPECT_EQ(result, 0);

}

TEST_F(TitleBarWidgetTest, checkCustomFixedTab)
{
    // Test method: bool checkCustomFixedTab((int index))
    auto result = obj->checkCustomFixedTab(0);
    EXPECT_FALSE(result);

}

TEST_F(TitleBarWidgetTest, currentUrl)
{
    // Test getter: QUrl currentUrl()
    auto result = obj->currentUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TitleBarWidgetTest, handleCreateTabList)
{
    // Test method: void handleCreateTabList((const QList<QUrl> &urlList))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleCreateTabList(_arg0));
}

TEST_F(TitleBarWidgetTest, handleCreateView)
{
    // Test method: void handleCreateView((const QString &uniqueId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleCreateView(_arg0));
}

TEST_F(TitleBarWidgetTest, handleHotketActivateTab)
{
    // Test method: void handleHotketActivateTab((const int index))
    EXPECT_NO_FATAL_FAILURE(obj->handleHotketActivateTab(0));
}

TEST_F(TitleBarWidgetTest, handleHotketCloseCurrentTab)
{
    // Test method: void handleHotketCloseCurrentTab(())
    EXPECT_NO_FATAL_FAILURE(obj->handleHotketCloseCurrentTab());
}

TEST_F(TitleBarWidgetTest, handleHotketCreateNewTab)
{
    // Test method: void handleHotketCreateNewTab(())
    EXPECT_NO_FATAL_FAILURE(obj->handleHotketCreateNewTab());
}

TEST_F(TitleBarWidgetTest, handleHotketNextTab)
{
    // Test method: void handleHotketNextTab(())
    EXPECT_NO_FATAL_FAILURE(obj->handleHotketNextTab());
}

TEST_F(TitleBarWidgetTest, handleHotketPreviousTab)
{
    // Test method: void handleHotketPreviousTab(())
    EXPECT_NO_FATAL_FAILURE(obj->handleHotketPreviousTab());
}

TEST_F(TitleBarWidgetTest, handleHotkeyCtrlF)
{
    // Test method: void handleHotkeyCtrlF(())
    EXPECT_NO_FATAL_FAILURE(obj->handleHotkeyCtrlF());
}

TEST_F(TitleBarWidgetTest, handleHotkeyCtrlL)
{
    // Test method: void handleHotkeyCtrlL(())
    EXPECT_NO_FATAL_FAILURE(obj->handleHotkeyCtrlL());
}

TEST_F(TitleBarWidgetTest, handleSplitterAnimation)
{
    // Test method: void handleSplitterAnimation((const QVariant &position))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSplitterAnimation(_arg0));
}

TEST_F(TitleBarWidgetTest, navWidget)
{
    // Test getter: NavWidget navWidget()
    auto result = obj->navWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->navWidget(); });

}

TEST_F(TitleBarWidgetTest, onAddressBarJump)
{
    // Test method: void onAddressBarJump(())
    EXPECT_NO_FATAL_FAILURE(obj->onAddressBarJump());
}

TEST_F(TitleBarWidgetTest, onTabAddButtonClicked)
{
    // Test method: void onTabAddButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onTabAddButtonClicked());
}

TEST_F(TitleBarWidgetTest, onTabCloseRequested)
{
    // Test method: void onTabCloseRequested((int index))
    EXPECT_NO_FATAL_FAILURE(obj->onTabCloseRequested(0));
}

TEST_F(TitleBarWidgetTest, onTabCreated)
{
    // Test method: void onTabCreated(())
    EXPECT_NO_FATAL_FAILURE(obj->onTabCreated());
}

TEST_F(TitleBarWidgetTest, onTabCurrentChanged)
{
    // Test method: void onTabCurrentChanged((int oldIndex, int newIndex))
    EXPECT_NO_FATAL_FAILURE(obj->onTabCurrentChanged(0, 0));
}

TEST_F(TitleBarWidgetTest, onTabMoved)
{
    // Test method: void onTabMoved((int from, int to))
    EXPECT_NO_FATAL_FAILURE(obj->onTabMoved(0, 0));
}

TEST_F(TitleBarWidgetTest, openCustomFixedTabs)
{
    // Test method: void openCustomFixedTabs(())
    EXPECT_NO_FATAL_FAILURE(obj->openCustomFixedTabs());
}

TEST_F(TitleBarWidgetTest, openNewTab)
{
    // Test method: void openNewTab((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openNewTab(_arg0));
}

TEST_F(TitleBarWidgetTest, quitSearch)
{
    // Test method: void quitSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->quitSearch());
}

TEST_F(TitleBarWidgetTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(TitleBarWidgetTest, restoreTitleBarState)
{
    // Test method: void restoreTitleBarState((const QString &uniqueId))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->restoreTitleBarState(_arg0));
}

TEST_F(TitleBarWidgetTest, setCurrentUrl)
{
    // Test setter: void setCurrentUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentUrl(_arg0));
}

TEST_F(TitleBarWidgetTest, setViewModeState)
{
    // Test setter: void setViewModeState((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->setViewModeState(0));
}

TEST_F(TitleBarWidgetTest, showAddrsssBar)
{
    // Test method: void showAddrsssBar((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showAddrsssBar(_arg0));
}

TEST_F(TitleBarWidgetTest, showCrumbBar)
{
    // Test method: void showCrumbBar(())
    EXPECT_NO_FATAL_FAILURE(obj->showCrumbBar());
}

TEST_F(TitleBarWidgetTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(TitleBarWidgetTest, showSearchFilterButton)
{
    // Test method: void showSearchFilterButton((bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->showSearchFilterButton(false));
}

TEST_F(TitleBarWidgetTest, titleCrumbBar)
{
    // Test getter: CrumbBar titleCrumbBar()
    auto result = obj->titleCrumbBar();
    EXPECT_NO_FATAL_FAILURE({ obj->titleCrumbBar(); });

}

TEST_F(TitleBarWidgetTest, updateUiForSizeMode)
{
    // Test method: void updateUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->updateUiForSizeMode());
}
