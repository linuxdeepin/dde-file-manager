/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#define private public
#define protected public

#include "views/dfilemanagerwindow_p.h"
#include "views/dfilemanagerwindow.h"
#include "dfmapplication.h"
#include "dapplication.h"
#include "app/filesignalmanager.h"
#include "singleton.h"
#include "app/define.h"
#include <QDesktopWidget>
#include "stub.h"

DFM_USE_NAMESPACE DCORE_USE_NAMESPACE


namespace  {
    static DFileManagerWindow *m_fileManagerWindow;
    class TestFileManagerWindow : public testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            m_fileManagerWindow = new DFileManagerWindow(DUrl(COMPUTER_ROOT));
            m_fileManagerWindow->openNewTab(DUrl(DFMMD_ROOT));
        }
        static void TearDownTestCase()
        {
            delete m_fileManagerWindow;
            m_fileManagerWindow = nullptr;
        }
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }
    };
}

TEST_F(TestFileManagerWindow, can_get_current_url)
{
    DUrl result = m_fileManagerWindow->currentUrl();
    DUrl expect = m_fileManagerWindow->d_func()->currentView ? m_fileManagerWindow->d_func()->currentView->rootUrl() : DUrl();
    EXPECT_EQ(expect, result);
}

TEST_F(TestFileManagerWindow, can_get_currentViewState)
{
    DFMBaseView::ViewState result = m_fileManagerWindow->currentViewState();
    DFMBaseView::ViewState expect = m_fileManagerWindow->d_func()->currentView ? m_fileManagerWindow->d_func()->currentView->viewState() : DFMBaseView::ViewIdle;
    EXPECT_EQ(expect, result);
}

TEST_F(TestFileManagerWindow, tst_isCurrentUrlSupportSearch)
{
    EXPECT_FALSE(m_fileManagerWindow->isCurrentUrlSupportSearch(DUrl(COMPUTER_ROOT)));
}

TEST_F(TestFileManagerWindow, tst_getToolBar)
{
    EXPECT_NE(nullptr, m_fileManagerWindow->getToolBar());
}

TEST_F(TestFileManagerWindow, tst_getFileView)
{
    EXPECT_NE(nullptr, m_fileManagerWindow->getFileView());
}

TEST_F(TestFileManagerWindow, tst_getLeftSideBar)
{
    EXPECT_NE(nullptr, m_fileManagerWindow->getLeftSideBar());
}

TEST_F(TestFileManagerWindow, tst_getSplitterPosition)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->getSplitterPosition());
}

TEST_F(TestFileManagerWindow, tst_setSplitterPosition)
{
    int position = 10;
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->setSplitterPosition(position));
}

TEST_F(TestFileManagerWindow, can_get_windowId)
{
    EXPECT_NE(0, m_fileManagerWindow->windowId());
}

TEST_F(TestFileManagerWindow, can_get_tabAddable)
{
    EXPECT_TRUE(m_fileManagerWindow->tabAddable());
}

TEST_F(TestFileManagerWindow, tst_hideRenameBar)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->hideRenameBar());
}

TEST_F(TestFileManagerWindow, tst_requestToSelectUrls)
{
    Stub stub;
    bool (*ut_isVisible)() = [](){return true;};
    stub.set(ADDR(QWidget, isVisible), ut_isVisible);

    m_fileManagerWindow->requestToSelectUrls();

    if (!m_fileManagerWindow->d_func()->renameBar) {
        m_fileManagerWindow->d_ptr->initRenameBar();
        m_fileManagerWindow->onReuqestCacheRenameBarState();
        m_fileManagerWindow->requestToSelectUrls();
    }
}

TEST_F(TestFileManagerWindow, tst_isAdvanceSearchBarVisible)
{
    EXPECT_FALSE(m_fileManagerWindow->isAdvanceSearchBarVisible());
}

TEST_F(TestFileManagerWindow, tst_toggleAdvanceSearchBar)
{
    m_fileManagerWindow->toggleAdvanceSearchBar();
}

TEST_F(TestFileManagerWindow, tst_showFilterButton)
{
    m_fileManagerWindow->showFilterButton();
}

TEST_F(TestFileManagerWindow, tst_moveCenter)
{
    QPoint pos(10, 10);

    QRect qr = m_fileManagerWindow->frameGeometry();
    qr.moveCenter(pos);

    m_fileManagerWindow->moveCenter(pos);

    EXPECT_EQ(qr.topLeft(), m_fileManagerWindow->pos());
}

TEST_F(TestFileManagerWindow, tst_moveTopRight)
{
    QRect pRect;
    pRect = qApp->desktop()->availableGeometry();
    int x = pRect.width() - m_fileManagerWindow->width();
    m_fileManagerWindow->moveTopRight();

    EXPECT_EQ(QPoint(x, 0), m_fileManagerWindow->pos());
}


TEST_F(TestFileManagerWindow, tst_moveCenterByRect)
{
    QRect rect(1, 1, 1, 1);

    QRect qr = m_fileManagerWindow->frameGeometry();
    qr.moveCenter(rect.center());

    m_fileManagerWindow->moveCenterByRect(rect);

    EXPECT_EQ(qr.topLeft(), m_fileManagerWindow->pos());
}


TEST_F(TestFileManagerWindow, tst_moveTopRightByRect)
{
    QRect rect(1, 1, 1, 1);

    int x = rect.x() + rect.width() - m_fileManagerWindow->width();

    m_fileManagerWindow->moveTopRightByRect(rect);

    EXPECT_EQ(QPoint(x, 0), m_fileManagerWindow->pos());
}

TEST_F(TestFileManagerWindow, tst_cd)
{    
    m_fileManagerWindow->d_func()->tabBar->m_currentIndex = -1;

    DUrl tstUrl(COMPUTER_ROOT);
    m_fileManagerWindow->cd(tstUrl);
    EXPECT_EQ(tstUrl, m_fileManagerWindow->currentUrl());
}

TEST_F(TestFileManagerWindow, tst_cdForTab)
{
    DUrl tstUrl(COMPUTER_ROOT);
    m_fileManagerWindow->openNewTab(tstUrl);
    EXPECT_EQ(true, m_fileManagerWindow->cdForTab(0, tstUrl));
}

TEST_F(TestFileManagerWindow, tst_cdForTabByView)
{
    DUrl tstUrl(COMPUTER_ROOT);
    EXPECT_FALSE(m_fileManagerWindow->cdForTabByView(nullptr, tstUrl));
    EXPECT_TRUE(m_fileManagerWindow->cdForTabByView(m_fileManagerWindow->getFileView(), tstUrl));
}

TEST_F(TestFileManagerWindow, tst_openNewTab)
{
    DUrl tstUrl;
    EXPECT_TRUE(m_fileManagerWindow->openNewTab(tstUrl));
}

TEST_F(TestFileManagerWindow, tst_switchToView)
{
    m_fileManagerWindow->openNewTab(DUrl(DFMROOT_ROOT));
    DFMBaseView *view = m_fileManagerWindow->getFileView();
    m_fileManagerWindow->openNewTab(DUrl(COMPUTER_ROOT));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->switchToView(view));
}

TEST_F(TestFileManagerWindow, tst_onTabAddableChanged)
{
    m_fileManagerWindow->onTabAddableChanged(false);
}

TEST_F(TestFileManagerWindow, tst_onCurrentTabChanged)
{
    m_fileManagerWindow->cdForTab(0, DUrl(COMPUTER_ROOT));
    m_fileManagerWindow->onCurrentTabChanged(0);
    EXPECT_EQ(DUrl(COMPUTER_ROOT), m_fileManagerWindow->getFileView()->rootUrl());

    m_fileManagerWindow->cdForTab(1, DUrl(DFMMD_ROOT));
    m_fileManagerWindow->onCurrentTabChanged(1);
    EXPECT_EQ(DUrl(DFMMD_ROOT), m_fileManagerWindow->getFileView()->rootUrl());
}

TEST_F(TestFileManagerWindow, tst_onRequestCloseTab)
{
    m_fileManagerWindow->onRequestCloseTab(0, true);
}

TEST_F(TestFileManagerWindow, tst_closeCurrentTab)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->closeCurrentTab(0));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->closeCurrentTab(m_fileManagerWindow->windowId()));
}

TEST_F(TestFileManagerWindow, tst_closeAllTabOfVault)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->closeAllTabOfVault(m_fileManagerWindow->windowId()));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->closeAllTabOfVault(0));
}

TEST_F(TestFileManagerWindow, can_show_hide_newTabButton)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->showNewTabButton());
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->hideNewTabButton());
}

TEST_F(TestFileManagerWindow, can_show_hide_emptyTrashButton)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->showEmptyTrashButton());
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->hideEmptyTrashButton());
}

//TEST_F(TestFileManagerWindow, tst_onNewTabButtonClicked)
//{
//    Stub stub;
//    bool (*ut_isValid)() = [](){return false;};
//    stub.set(ADDR(DUrl, isValid), ut_isValid);

//    DUrl url(COMPUTER_ROOT);
//    DFMApplication::instance()->setAppAttribute(DFMApplication::AA_UrlOfNewTab, url);
//    m_fileManagerWindow->onNewTabButtonClicked();
//}

TEST_F(TestFileManagerWindow, tst_requestEmptyTrashFiles)
{
    Stub stub;
    void (*ut_clearTrash)() = [](){};
    stub.set(ADDR(DFMGlobal, clearTrash), ut_clearTrash);

    m_fileManagerWindow->requestEmptyTrashFiles();
}

TEST_F(TestFileManagerWindow, tst_onTrashStateChanged)
{
    m_fileManagerWindow->cd(DUrl::fromTrashFile("/"));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onTrashStateChanged());

    m_fileManagerWindow->cd(DUrl::fromTrashFile("/home/"));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onTrashStateChanged());
}

TEST_F(TestFileManagerWindow, tst_onShowRenameBar)
{
    DUrlList urls;
    urls << DUrl(COMPUTER_ROOT);
    auto event = dMakeEventPointer<DFMUrlListBaseEvent>(m_fileManagerWindow, urls);
    m_fileManagerWindow->onShowRenameBar(*event);
}

TEST_F(TestFileManagerWindow, tst_onTabBarCurrentIndexChange)
{
    if (!m_fileManagerWindow->d_func()->renameBar) {
        m_fileManagerWindow->d_ptr->initRenameBar();
    }

    Stub stub;
    bool (*ut_isRenameBarVisible)() = [](){return true;};
    stub.set(ADDR(DFileManagerWindowPrivate, isRenameBarVisible), ut_isRenameBarVisible);

    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onTabBarCurrentIndexChange(0));
}

TEST_F(TestFileManagerWindow, tst_onReuqestCacheRenameBarState)
{
    m_fileManagerWindow->d_ptr->initRenameBar();
    m_fileManagerWindow->onReuqestCacheRenameBarState();
}

TEST_F(TestFileManagerWindow, can_RequestRedirectUrl)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestRedirectUrl(DUrl("/"), DUrl("/home")));
}

TEST_F(TestFileManagerWindow, can_RequestCloseTabByUrl)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(DUrl(TRASH_ROOT)));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(DUrl(COMPUTER_ROOT)));
    m_fileManagerWindow->openNewTab(DUrl(COMPUTER_ROOT));
    m_fileManagerWindow->openNewTab(DUrl(COMPUTER_ROOT));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(m_fileManagerWindow->currentUrl()));
}

TEST_F(TestFileManagerWindow, tst_show_close)
{
    m_fileManagerWindow->show();
    m_fileManagerWindow->close();
}

TEST_F(TestFileManagerWindow, tst_show_hide)
{
    m_fileManagerWindow->show();
    m_fileManagerWindow->hide();
}

TEST_F(TestFileManagerWindow, tst_doubleClick_event)
{
    QMouseEvent event(QEvent::MouseButtonDblClick,{0,0},Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    m_fileManagerWindow->mouseDoubleClickEvent(&event);
}

TEST_F(TestFileManagerWindow, tst_move_event)
{
    QMoveEvent event(QPoint(0, 0), QPoint(1, 1));
    m_fileManagerWindow->moveEvent(&event);
}

TEST_F(TestFileManagerWindow, tst_keyPress_event)
{
    QKeyEvent keyPressEvt_Tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_fileManagerWindow->keyPressEvent(&keyPressEvt_Tab);
}

TEST_F(TestFileManagerWindow, tst_filter_event)
{
    DTitlebar *watched = m_fileManagerWindow->titlebar();
    QKeyEvent keyPressEvt_Tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_fileManagerWindow->eventFilter((QObject*)watched, &keyPressEvt_Tab);

    DFMBaseView *temp = m_fileManagerWindow->d_func()->currentView;
    m_fileManagerWindow->d_func()->currentView = nullptr;
    m_fileManagerWindow->eventFilter(nullptr, &keyPressEvt_Tab);

    m_fileManagerWindow->d_func()->currentView = temp;
    m_fileManagerWindow->eventFilter(nullptr, &keyPressEvt_Tab);

    QKeyEvent keyReleaseEvt_Tab(QEvent::KeyRelease, Qt::Key_Tab, Qt::NoModifier);
    m_fileManagerWindow->eventFilter(nullptr, &keyPressEvt_Tab);
}

TEST_F(TestFileManagerWindow, tst_resize_event)
{
    QResizeEvent event(QSize(100, 100), QSize(50, 50));
    m_fileManagerWindow->resizeEvent(&event);
}

TEST_F(TestFileManagerWindow, tst_fm_event)
{
    QSharedPointer<DFMEvent> event = QSharedPointer<DFMEvent>(new DFMEvent);
    event->setType(DFMEvent::Back);
    m_fileManagerWindow->fmEvent(event);

    event->setType(DFMEvent::Forward);
    m_fileManagerWindow->fmEvent(event);

    event->setType(DFMEvent::OpenNewTab);
    m_fileManagerWindow->fmEvent(event);

    event->setType(DFMEvent::UnknowType);
    m_fileManagerWindow->fmEvent(event);
}

TEST_F(TestFileManagerWindow, tst_get_object)
{
    EXPECT_NE(nullptr, m_fileManagerWindow->object());
}

TEST_F(TestFileManagerWindow, tst_handle_newView)
{
    m_fileManagerWindow->handleNewView(nullptr);
}

TEST_F(TestFileManagerWindow, can_initRenameBarState)
{
    m_fileManagerWindow->initRenameBarState();

    DFileManagerWindow::flagForNewWindowFromTab.store(true, std::memory_order_seq_cst);
    m_fileManagerWindow->initRenameBarState();
}

TEST_F(TestFileManagerWindow, can_initConnect)
{
    emit m_fileManagerWindow->d_func()->toolbar->detailButtonClicked();

    QList<DUrl> urlList;
    urlList.append(DUrl("/home"));
    emit m_fileManagerWindow->selectUrlChanged(urlList);
}
