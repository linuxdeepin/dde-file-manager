/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-28

 */


#include <gtest/gtest.h>

#include "videopreview.h"

#include "durl.h"

DFM_BEGIN_NAMESPACE

class TestVideoPreview : public testing::Test
{
public:
    void SetUp() override
    {
        m_videoPreview = new VideoPreview(nullptr);

        m_url = DUrl("file:///usr/share/dde-introduction/demo.mp4");
    }

    void TearDown() override
    {
        delete m_videoPreview;
        m_videoPreview = nullptr;
    }

public:
    VideoPreview * m_videoPreview;

    DUrl m_url;
};

TEST_F(TestVideoPreview, set_file_url)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
}

TEST_F(TestVideoPreview, get_file_url)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
    EXPECT_TRUE(!m_videoPreview->fileUrl().isValid());
}

TEST_F(TestVideoPreview, get_content_widget)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_videoPreview->contentWidget());
}

TEST_F(TestVideoPreview, get_statusbar_widget)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_videoPreview->statusBarWidget());
}

TEST_F(TestVideoPreview, get_show_statusbar_separator)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_videoPreview->showStatusBarSeparator());
}

TEST_F(TestVideoPreview, get_statusbar_widget_alignment)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_videoPreview->statusBarWidgetAlignment());
}


DFM_END_NAMESPACE
