/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-26

 */

#include <gtest/gtest.h>

#include <QDir>

#include "musicpreview.h"

DFM_BEGIN_NAMESPACE
namespace {
class TestMusicPreview : public testing::Test
{
public:

    void SetUp() override
    {
        m_musicPerview = new MusicPreview(nullptr);
        m_url = DUrl("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/tests/bensound-sunny.mp3");
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


TEST_F(TestMusicPreview, set_file_url)
{
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

DFM_END_NAMESPACE
