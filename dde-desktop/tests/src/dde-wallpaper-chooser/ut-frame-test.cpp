#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public

#include "../dde-wallpaper-chooser/frame.h"



using namespace testing;

namespace {
    class FrameTest : public Test{
    public:
        FrameTest() : Test(){

        }

        virtual void SetUp() override{
            frame = new Frame("screen-name");
        }

        virtual void TearDown() override{
            delete frame;
        }

        Frame* frame;
    };
}

TEST_F(FrameTest,show)
{
    bool isFfirst = frame->isHidden();
    frame->show();
    bool isSecond = frame->isHidden();
    EXPECT_EQ(isFfirst, true);
    EXPECT_NE(isFfirst, isSecond);
}


TEST_F(FrameTest,hide)
{
    frame->show();
    bool First = frame->isHidden();
    frame->hide();
    bool Second = frame->isHidden();
    EXPECT_EQ(First,false);
    EXPECT_NE(First,Second);
}

TEST_F(FrameTest,onItemPressed)
{
    frame->onItemPressed("close");
    EXPECT_EQ(frame->m_desktopWallpaper,frame->m_lockWallpaper);
}

TEST_F(FrameTest,getWallpaperSlideShow)
{

    frame->setWallpaperSlideShow("hh");
    QString temp = frame->getWallpaperSlideShow();
    EXPECT_FALSE(temp.isEmpty());

}

TEST_F(FrameTest,onItemButtonClicked)
{
    frame->onItemPressed("close");
    EXPECT_FALSE(frame->m_desktopWallpaper.isEmpty());
    frame->onItemButtonClicked("lock-screen");
    EXPECT_TRUE(frame->isHidden());
}
