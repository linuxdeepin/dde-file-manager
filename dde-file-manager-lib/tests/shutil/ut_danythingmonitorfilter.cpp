#include "shutil/danythingmonitorfilter.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QDir>

namespace  {
    class TestDAnythingMonitorFilter: public testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }

    };
}

// 用户的图片，视频是可以打tag 的
TEST_F(TestDAnythingMonitorFilter, can_tag_the_user_pictures)
{
    QString usrPictures = QDir::homePath() + "/Pictures/Wallpapers";
    EXPECT_TRUE( DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(usrPictures.toUtf8()));
}

// 用户的图片，视频是可以打tag 的
TEST_F(TestDAnythingMonitorFilter, can_tag_the_user_pictures_abc_123jpg)
{
    QString usrPictures = QDir::homePath() + "/Pictures/Wallpapers/abc-123.jpg";
    EXPECT_TRUE( DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(usrPictures.toUtf8()));
}

// 一般的文件不可以打tag 的
TEST_F(TestDAnythingMonitorFilter, normal_file_cant_taged)
{
    QString usrPictures = QDir::homePath() + "/Pictures/Wallpapers";
    EXPECT_FALSE( DAnythingMonitorFilter::instance()->whetherFilterCurrentPath("etc/apt/sources.list"));
}

// 不存在的文件不可以tag
TEST_F(TestDAnythingMonitorFilter, not_exited_file_cant_taged)
{
    QString usrPictures = QDir::homePath() + "/Pictures/Wallpapers";
    EXPECT_FALSE( DAnythingMonitorFilter::instance()->whetherFilterCurrentPath("etc/apt/no-sources.list"));
}
