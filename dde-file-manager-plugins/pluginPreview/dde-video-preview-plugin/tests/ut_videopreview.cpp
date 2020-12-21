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

#include "stub.h"
#include "addr_pri.h"

#include <gtest/gtest.h>

#include "videopreview.h"

#include <dimagebutton.h>

#include "durl.h"
DFM_USE_NAMESPACE

class TestVideoPreview : public testing::Test
{
public:
    void SetUp() override
    {
        m_videoPreview = new VideoPreview(nullptr);

        m_url = DUrl("file:/usr/share/dde-introduction/demo.mp4");

    }

    void TearDown() override
    {
        delete m_videoPreview;
        m_videoPreview = nullptr;
    }

public:
    VideoPreview * m_videoPreview;

    VideoWidget * m_videoWidget;

    DUrl m_url;
};

TEST_F(TestVideoPreview, set_file_url)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
}

TEST_F(TestVideoPreview, get_file_url)
{
    EXPECT_TRUE(m_videoPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_videoPreview->fileUrl().isValid());
}

TEST_F(TestVideoPreview, get_content_widget)
{
    EXPECT_TRUE(m_videoPreview->contentWidget() != nullptr);
}

TEST_F(TestVideoPreview, get_statusbar_widget)
{
    EXPECT_TRUE(m_videoPreview->statusBarWidget() != nullptr);
}

TEST_F(TestVideoPreview, get_show_statusbar_separator)
{
    EXPECT_FALSE(m_videoPreview->showStatusBarSeparator());
}

TEST_F(TestVideoPreview, get_statusbar_widget_alignment)
{
    bool temp = static_cast<int>(m_videoPreview->statusBarWidgetAlignment()) > 0;
    EXPECT_FALSE(temp);
}

TEST_F(TestVideoPreview, use_play)
{
    m_videoPreview->play();
}

typedef QPointer<VideoWidget> PVideoWidget;
ACCESS_PRIVATE_FIELD(VideoPreview,  PVideoWidget, playerWidget);
PVideoWidget PrivateplayerWidget(VideoPreview * videoView){
    return access_private_field::VideoPreviewplayerWidget(*videoView);
};
TEST_F(TestVideoPreview, get_sizeHint)
{
    PrivateplayerWidget(m_videoPreview)->sizeHint();
}

TEST_F(TestVideoPreview, emit_stateChanged)
{
    emit PrivateplayerWidget(m_videoPreview)->engine().stateChanged();
}

typedef QPointer<VideoStatusBar> PVideoStatusBar;
ACCESS_PRIVATE_FIELD(VideoPreview,  PVideoStatusBar, statusBar);
PVideoStatusBar PrivatestatusBar(VideoPreview * videoView){
    return access_private_field::VideoPreviewstatusBar(*videoView);
};
TEST_F(TestVideoPreview, emit_elapsedChanged)
{
    emit PrivatestatusBar(m_videoPreview)->slider->sliderPressed();
    emit PrivateplayerWidget(m_videoPreview)->engine().elapsedChanged();
    emit PrivatestatusBar(m_videoPreview)->slider->sliderReleased();
    emit PrivateplayerWidget(m_videoPreview)->engine().elapsedChanged();
}

TEST_F(TestVideoPreview, emit_valueChanged)
{
    emit PrivatestatusBar(m_videoPreview)->slider->valueChanged(10);
}

TEST_F(TestVideoPreview, use_pause)
{
    m_videoPreview->pause();
}

TEST_F(TestVideoPreview, use_stop)
{
    m_videoPreview->stop();
}

TEST_F(TestVideoPreview, use_DoneCurrent)
{
    m_videoPreview->DoneCurrent();
}
