#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "views/dfilemanagerwindow.h"
#define private public
#define protected public
#include "views/windowmanager.h"

namespace  {
    class WindowManagerTest : public testing::Test
    {
    public:
        WindowManager wm;
        virtual void SetUp() override
        {

        }

        virtual void TearDown() override
        {
            WindowManager::m_count = 0;
            for (const QWidget *w : WindowManager::m_windows.keys()){
                    delete w;
            }
            WindowManager::m_windows.clear();
        }
    };
}

TEST_F(WindowManagerTest,save_window_state)
{
    DFileManagerWindow win;
    win.resize(500,500);
    EXPECT_NO_FATAL_FAILURE(wm.saveWindowState(&win));
}

TEST_F(WindowManagerTest,load_window_state)
{
    DFileManagerWindow win;
    win.resize(600,600);

    wm.saveWindowState(&win);
    DFileManagerWindow win2;
    wm.loadWindowState(&win2);
    EXPECT_EQ(win.size(),win2.size());
}

TEST_F(WindowManagerTest,show_new_window)
{
    EXPECT_EQ(0,wm.m_windows.size());

    wm.showNewWindow(DUrl("file:///home"),true);
    EXPECT_EQ(1,wm.m_windows.size());
}

TEST_F(WindowManagerTest,show_new_window_repate_true)
{
    EXPECT_EQ(0,wm.m_windows.size());

    wm.showNewWindow(DUrl("file:///home"),true);
    EXPECT_EQ(1,wm.m_windows.size());

    wm.showNewWindow(DUrl("file:///home"),true);
    EXPECT_EQ(2,wm.m_windows.size());
}

TEST_F(WindowManagerTest,show_new_window_repate_false)
{
    EXPECT_EQ(0,wm.m_windows.size());

    wm.showNewWindow(DUrl("file:///home"),true);
    EXPECT_EQ(1,wm.m_windows.size());

    wm.showNewWindow(DUrl("file:///home"),false);
    EXPECT_EQ(1,wm.m_windows.size());
}

TEST_F(WindowManagerTest,get_windowid)
{
    wm.showNewWindow(DUrl("file:///home"),true);
    ASSERT_EQ(1,wm.m_windows.size());
    EXPECT_EQ(wm.m_windows.begin().key()->winId(),wm.getWindowId(wm.m_windows.begin().key()));
}

TEST_F(WindowManagerTest,get_window_by_id)
{
    wm.showNewWindow(DUrl("file:///home"),true);
    ASSERT_EQ(1,wm.m_windows.size());
    EXPECT_EQ(wm.m_windows.begin().key(),wm.getWindowById(wm.m_windows.begin().key()->winId()));
}

TEST_F(WindowManagerTest,tab_addable_by_winId_null)
{
    EXPECT_FALSE(wm.tabAddableByWinId(-1));
}

TEST_F(WindowManagerTest,tab_addable_by_winId)
{
    wm.showNewWindow(DUrl("file:///home"),true);
    ASSERT_EQ(1,wm.m_windows.size());
    EXPECT_EQ(((DFileManagerWindow*)wm.m_windows.begin().key())->tabAddable(),
              wm.tabAddableByWinId(wm.m_windows.begin().key()->winId()));
}
