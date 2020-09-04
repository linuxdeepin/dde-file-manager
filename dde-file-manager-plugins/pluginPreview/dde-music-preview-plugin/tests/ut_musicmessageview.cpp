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

#ifdef GTEST
#define private public
#define protected public
#endif

#include "musicmessageview.h"
#include "durl.h"
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>

namespace {
class TestMusicMessageView : public testing::Test
{
public:

    void SetUp() override
    {
        m_url = "file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/tests/bensound-sunny.mp3";
        m_musicMessageView = new MusicMessageView(m_url);
    }

    void TearDown() override
    {
        delete m_musicMessageView;
        m_musicMessageView = nullptr;
    }

public:
    MusicMessageView *m_musicMessageView;

    QString m_url;
};
}

TEST_F(TestMusicMessageView, can_tag_open_music_file)
{
    EXPECT_TRUE(m_musicMessageView->tagOpenMusicFile(m_url).title.toStdString().c_str());
    EXPECT_TRUE(m_musicMessageView->tagOpenMusicFile(m_url).album.toStdString().c_str());
    EXPECT_TRUE(m_musicMessageView->tagOpenMusicFile(m_url).artist.toStdString().c_str());
}

TEST_F(TestMusicMessageView, can_characterEncodingTransform)
{
    MediaMeta meta;
    QUrl url(m_url);
    TagLib::FileRef f(url.toLocalFile().toLocal8Bit());

    TagLib::Tag *tag = f.tag();
    m_musicMessageView->characterEncodingTransform(meta, static_cast<void*>(tag));
    EXPECT_TRUE(meta.title.toStdString().c_str());
    EXPECT_TRUE(meta.album.toStdString().c_str());
    EXPECT_TRUE(meta.artist.toStdString().c_str());
}

TEST_F(TestMusicMessageView, can_detect_encodings)
{
    MediaMeta meta;
    QUrl url(m_url);
    TagLib::FileRef f(url.toLocalFile().toLocal8Bit());
    TagLib::Tag *tag = f.tag();
    bool encode = true;
    encode &= tag->title().isNull() ? true : tag->title().isLatin1();
    encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
    encode &= tag->album().isNull() ? true : tag->album().isLatin1();

    QByteArray detectByte;
    QByteArray detectCodec;
    if (encode)
    {
        if (detectCodec.isEmpty())
        {
            detectByte += tag->title().toCString();
            detectByte += tag->artist().toCString();
            detectByte += tag->album().toCString();
            EXPECT_TRUE(!m_musicMessageView->detectEncodings(detectByte).isEmpty());
        }
    }
}

TEST_F(TestMusicMessageView, can_is_chinese)
{
    MediaMeta meta;
    QUrl url(m_url);
    TagLib::FileRef f(url.toLocalFile().toLocal8Bit());
    TagLib::Tag *tag = f.tag();
    bool encode = true;
    encode &= tag->title().isNull() ? true : tag->title().isLatin1();
    encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
    encode &= tag->album().isNull() ? true : tag->album().isLatin1();

    QByteArray detectByte;
    QByteArray detectCodec;
    if (encode)
    {
        if (detectCodec.isEmpty())
        {
            detectByte += tag->title().toCString();
            detectByte += tag->artist().toCString();
            detectByte += tag->album().toCString();
            auto allDetectCodecs = m_musicMessageView->detectEncodings(detectByte);
            auto localeCode = m_musicMessageView->localeCodes.value(QLocale::system().name());

            for (auto curDetext : allDetectCodecs)
            {
                if (curDetext == "Big5" || curDetext == localeCode)
                {
                    detectCodec = curDetext;
                    break;
                }
            }
            if (detectCodec.isEmpty())
                detectCodec = allDetectCodecs.value(0);

            QString curStr = QString::fromLocal8Bit(tag->title().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->artist().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->album().toCString());
            for (auto ch : curStr)
            {
                EXPECT_TRUE(m_musicMessageView->isChinese(ch));
                if (m_musicMessageView->isChinese(ch))
                {
                    detectCodec = "GB18030";
                    break;
                }
            }
        }
    }
}

TEST_F(TestMusicMessageView, can_play)
{
    emit m_musicMessageView->m_player->mediaStatusChanged(QMediaPlayer::LoadedMedia);
}
