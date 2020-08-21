#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "views/dfilemanagerwindow.h"
#include "dfmapplication.h"
#include "dapplication.h"
#include "app/filesignalmanager.h"
#include "singleton.h"
#include "app/define.h"
#include <QDesktopWidget>


DFM_USE_NAMESPACE
DCORE_USE_NAMESPACE

namespace  {
    class TestFileManagerWindow : public testing::Test
    {
    public:
        DFileManagerWindow *m_fileManagerWindow;
        virtual void SetUp() override
        {
            m_fileManagerWindow = new DFileManagerWindow(DUrl(COMPUTER_ROOT));
            m_fileManagerWindow->openNewTab(DUrl(DFMMD_ROOT));
            std::cout << "start DFileManagerWindow" << std::endl;
        }

        virtual void TearDown() override
        {
            delete m_fileManagerWindow;
            m_fileManagerWindow = nullptr;
            std::cout << "end DFileManagerWindow" << std::endl;
        }
    };
}

TEST_F(TestFileManagerWindow, can_get_currentViewState)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->currentViewState());
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
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->requestToSelectUrls());
}

TEST_F(TestFileManagerWindow, tst_isAdvanceSearchBarVisible)
{
    EXPECT_FALSE(m_fileManagerWindow->isAdvanceSearchBarVisible());
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

TEST_F(TestFileManagerWindow, tst_onNewTabButtonClicked)
{
    DUrl url(COMPUTER_ROOT);
    DFMApplication::instance()->setAppAttribute(DFMApplication::AA_UrlOfNewTab, url);
    m_fileManagerWindow->onNewTabButtonClicked();

    EXPECT_EQ(url, m_fileManagerWindow->currentUrl());
}

TEST_F(TestFileManagerWindow, tst_onTrashStateChanged)
{
    m_fileManagerWindow->cd(DUrl::fromTrashFile("/"));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onTrashStateChanged());
}

TEST_F(TestFileManagerWindow, tst_onShowRenameBar)
{
    DUrlList urls;
    urls << DUrl(COMPUTER_ROOT);
    const DFMUrlListBaseEvent *event = dMakeEventPointer<DFMUrlListBaseEvent>(m_fileManagerWindow, urls).data();
    m_fileManagerWindow->onShowRenameBar(*event);
}

TEST_F(TestFileManagerWindow, tst_onTabBarCurrentIndexChange)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onTabBarCurrentIndexChange(0));
}

TEST_F(TestFileManagerWindow, can_RequestRedirectUrl)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestRedirectUrl(DUrl(), DUrl()));
}

TEST_F(TestFileManagerWindow, can_RequestCloseTabByUrl)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(DUrl(TRASH_ROOT)));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(DUrl(COMPUTER_ROOT)));
    m_fileManagerWindow->openNewTab(DUrl(COMPUTER_ROOT));
    m_fileManagerWindow->openNewTab(DUrl(COMPUTER_ROOT));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(m_fileManagerWindow->currentUrl()));
}
