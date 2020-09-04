#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public

#include "../dde-wallpaper-chooser/frame.h"

using namespace testing;

namespace {
    class FrameTest : public Test {
    public:
        FrameTest() : Test()
        {

        }

        virtual void SetUp() override
        {
            m_frame = new Frame("screen-name");
        }

        virtual void TearDown() override
        {
            delete m_frame;
        }

        Frame *m_frame = nullptr;
    };
}

TEST_F(FrameTest, show)
{
    bool bfirst = m_frame->isHidden();
    m_frame->show();
    bool bsecond = m_frame->isHidden();
    EXPECT_EQ(bfirst, true);
    EXPECT_NE(bfirst, bsecond);
}


TEST_F(FrameTest, hide)
{
    m_frame->show();
    bool bfirst = m_frame->isHidden();
    m_frame->hide();
    bool bsecond = m_frame->isHidden();
    EXPECT_EQ(bfirst, false);
    EXPECT_NE(bfirst, bsecond);
}

TEST_F(FrameTest, on_item_ispressed)
{
    m_frame->onItemPressed("close");
    EXPECT_EQ(m_frame->m_desktopWallpaper, m_frame->m_lockWallpaper);
}

TEST_F(FrameTest, get_wallpaperslideshow)
{
    m_frame->setWallpaperSlideShow("hh");
    QString str = m_frame->getWallpaperSlideShow();
    EXPECT_FALSE(str.isEmpty());
}

TEST_F(FrameTest, on_itembutton_isclicked)
{
    m_frame->onItemPressed("close");
    EXPECT_FALSE(m_frame->m_desktopWallpaper.isEmpty());
    m_frame->onItemButtonClicked("lock-screen");
    EXPECT_TRUE(m_frame->isHidden());
}
