// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDir>
#include <QPushButton>
#include <QMediaPlayer>
#include <QTimer>

#include "musicpreview.h"
#include "toolbarframe.h"
#include "stub.h"
#include "addr_pri.h"

DFM_USE_NAMESPACE
namespace {
class TestMusicPreview : public testing::Test
{
public:

    void SetUp() override
    {
        m_musicPerview = new MusicPreview(nullptr);
        m_url = DUrl("file:/usr/share/music/bensound-sunny.mp3");
    }

    void TearDown() override
    {
        delete m_musicPerview;
        m_musicPerview = nullptr;
    }

public:
    MusicPreview *m_musicPerview;

    DUrl m_url;
};
}

#ifndef __arm__
TEST_F(TestMusicPreview, set_file_url)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));

    bool (*st_isLocalFile)() = [](){
        return false;
    };
    Stub stub;
    stub.set(&QUrl::isLocalFile, st_isLocalFile);
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    stub.reset(&QUrl::isLocalFile);

    bool (*st_canPreview)(const DUrl &url) = [](const DUrl &url){
        return false;
    };
    stub.set(&MusicPreview::canPreview, st_canPreview);
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
}

TEST_F(TestMusicPreview, set_can_preview)
{
    EXPECT_TRUE(m_musicPerview->canPreview(m_url));
}

TEST_F(TestMusicPreview, get_file_url)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    EXPECT_STREQ(m_musicPerview->fileUrl().toLocalFile().toStdString().c_str(), m_url.toLocalFile().toStdString().c_str());
}

TEST_F(TestMusicPreview, get_content_widget)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    EXPECT_TRUE(m_musicPerview->contentWidget());
}

TEST_F(TestMusicPreview, get_statusbar_widget)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    EXPECT_TRUE(m_musicPerview->statusBarWidget());
}

TEST_F(TestMusicPreview, use_play)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    m_musicPerview->play();
}

TEST_F(TestMusicPreview, use_pause)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    m_musicPerview->play();
    m_musicPerview->pause();
}

TEST_F(TestMusicPreview, use_stop)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    m_musicPerview->play();
    m_musicPerview->stop();
}

ACCESS_PRIVATE_FIELD(MusicPreview, QPointer<ToolBarFrame>, m_statusBarFrame)
QPointer<ToolBarFrame> PrivateStausBarFrame(MusicPreview * musePreview)
{
    return access_private_field::MusicPreviewm_statusBarFrame(*musePreview);
}
ACCESS_PRIVATE_FIELD(ToolBarFrame, QPushButton*, m_playControlButton)
QPushButton* PrivateplayControlButton(ToolBarFrame * toolBarFrame)
{
    return access_private_field::ToolBarFramem_playControlButton(*toolBarFrame);
}
TEST_F(TestMusicPreview, use_onPlayControlButtonClicked)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    PrivateplayControlButton(PrivateStausBarFrame(m_musicPerview).data())->clicked();
}

ACCESS_PRIVATE_FIELD(ToolBarFrame, QMediaPlayer*, m_player)
QMediaPlayer* Privateplayer(ToolBarFrame * toolBarFrame)
{
    return access_private_field::ToolBarFramem_player(*toolBarFrame);
}
TEST_F(TestMusicPreview, use_onPlayDurationChanged)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    Privateplayer(PrivateStausBarFrame(m_musicPerview).data())->durationChanged(5);
}


ACCESS_PRIVATE_FIELD(ToolBarFrame, QTimer*, m_updateProgressTimer)
QTimer* PrivateupdateProgressTimer(ToolBarFrame * toolBarFrame)
{
    return access_private_field::ToolBarFramem_updateProgressTimer(*toolBarFrame);
}
TEST_F(TestMusicPreview, use_updateProgress)
{
    EXPECT_TRUE(m_musicPerview->setFileUrl(m_url));
    PrivateupdateProgressTimer(PrivateStausBarFrame(m_musicPerview).data())->timeout({});
}
#endif

