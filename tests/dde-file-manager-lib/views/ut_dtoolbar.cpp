/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QPushButton>

#include "views/historystack.h"
#include "views/dfilemanagerwindow.h"
#define private public
#include "views/dtoolbar.h"
#include "views/windowmanager.h"

TEST(DToolBarTest,init)
{
    DToolBar w;
    EXPECT_NE(nullptr,w.m_addressToolBar);
    EXPECT_NE(nullptr,w.m_backButton);
    EXPECT_NE(nullptr,w.m_forwardButton);
    EXPECT_NE(nullptr,w.m_searchButton);
    EXPECT_NE(nullptr,w.m_detailButton);
    EXPECT_NE(nullptr,w.m_contollerToolBar);
    EXPECT_NE(nullptr,w.m_contollerToolBarContentLayout);
    EXPECT_NE(nullptr,w.m_crumbWidget);
}

TEST(DToolBarTest,get_crumb_widget)
{
    DToolBar w;
    EXPECT_EQ(w.getCrumbWidget(),w.m_crumbWidget);
}

TEST(DToolBarTest,add_history_stack)
{
    DToolBar w;
    int org = w.m_navStacks.size();
    w.addHistoryStack();
    EXPECT_EQ(org + 1,w.m_navStacks.size());
}

TEST(DToolBarTest,remove_navStack)
{
    DToolBar w;
    w.addHistoryStack();
    w.addHistoryStack();
    w.removeNavStackAt(1);
    EXPECT_EQ(1,w.m_navStacks.size());
    w.switchHistoryStack(0);
}

TEST(DToolBarTest,switch_navStack)
{
    DToolBar w;
    w.addHistoryStack();
    w.addHistoryStack();
    w.switchHistoryStack(1);
    EXPECT_EQ(2,w.m_navStacks.size());
}

TEST(DToolBarTest,nav_stack_count)
{
    DToolBar w;
    EXPECT_EQ(w.m_navStacks.size(),w.navStackCount());
    w.addHistoryStack();
    EXPECT_EQ(w.m_navStacks.size(),w.navStackCount());
}

TEST(DToolBarTest,set_custom_action_list)
{
    DToolBar w;
    QAction ac("test");
    bool ok = false;
    QEventLoop loop;
    QObject::connect(&ac,&QAction::triggered,&ac,[&ok,&loop](){
        ok = true;
        loop.exit();
    },Qt::QueuedConnection);
    w.setCustomActionList({&ac});
    ASSERT_EQ(1,w.m_actionList.size());
    ASSERT_EQ(&ac, w.m_actionList.at(0));

    w.triggerActionByIndex(0);
    loop.exec();
    EXPECT_EQ(true,ok);
}

TEST(DToolBarTest,toggle_search_buttonstate_true)
{
    DToolBar w;
    w.toggleSearchButtonState(true);
    EXPECT_EQ(true,w.m_searchButtonAsbState);
}

TEST(DToolBarTest,search_bartext_entered_empty)
{
    DToolBar w;
    EXPECT_NO_FATAL_FAILURE(w.searchBarTextEntered(""));
}

TEST(DToolBarTest,search_bartext_entered_none_window)
{
    DToolBar w;
    QString homePath = "file:///home";
    EXPECT_NO_FATAL_FAILURE(w.searchBarTextEntered(homePath););
}

TEST(DToolBarTest,search_bartext_entered_window)
{
    DFileManagerWindow wm;
    DToolBar w(&wm);
    QString homePath = "file:///home";
    wm.cd(homePath);

    w.searchBarTextEntered("/usr");
    EXPECT_EQ("/usr", wm.currentUrl().toLocalFile());
}

TEST(DToolBarTest,moveNavStacks)
{
    DToolBar w;
    w.m_navStacks.clear();
    w.m_navStacks.append(0);
    std::shared_ptr<HistoryStack> h1(new HistoryStack(0));
    w.m_navStacks.append(h1);
    EXPECT_EQ(2, w.navStackCount());

    w.moveNavStacks(0,1);
    EXPECT_EQ(w.m_navStacks[0], h1);
    EXPECT_EQ(w.m_navStacks[1], nullptr);

    w.moveNavStacks(1,0);
    EXPECT_EQ(w.m_navStacks[1], h1);
    EXPECT_EQ(w.m_navStacks[0], nullptr);

    w.moveNavStacks(0,0);
    EXPECT_EQ(w.m_navStacks[1], h1);
    EXPECT_EQ(w.m_navStacks[0], nullptr);
}

TEST(DToolBarTest,showFilterButton)
{
    DToolBar w;
    w.show();
    w.m_searchButton->setVisible(false);
    w.setSearchButtonVisible(true);
    EXPECT_TRUE(w.m_searchButton->isVisible());

    w.m_searchButton->setVisible(true);
    w.setSearchButtonVisible(true);
    EXPECT_TRUE(w.m_searchButton->isVisible());
}

TEST(DToolBarTest,handleHotkeyCtrlF)
{
    DFileManagerWindow wm;
    DToolBar w(&wm);
    WindowManager::m_windows.insert(w.topLevelWidget(),w.winId());
    EXPECT_NO_FATAL_FAILURE(w.handleHotkeyCtrlF(w.winId()));
    WindowManager::m_windows.remove(w.topLevelWidget());
    EXPECT_NO_FATAL_FAILURE(w.handleHotkeyCtrlF(w.winId()));
}

TEST(DToolBarTest,handleHotkeyCtrlL)
{
    DFileManagerWindow wm;
    DToolBar w(&wm);
    WindowManager::m_windows.insert(w.topLevelWidget(),w.winId());
    EXPECT_NO_FATAL_FAILURE(w.handleHotkeyCtrlL(w.winId()));
    WindowManager::m_windows.remove(w.topLevelWidget());
    EXPECT_NO_FATAL_FAILURE(w.handleHotkeyCtrlL(w.winId()));
}

TEST(DToolBarTest,searchBarActivated_Deactivated)
{
    DToolBar w;
    w.m_searchButton->setHidden(false);
    w.searchBarActivated();
    w.searchBarDeactivated();
}
