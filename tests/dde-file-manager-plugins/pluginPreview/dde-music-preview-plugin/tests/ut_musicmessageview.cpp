/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version..
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <gtest/gtest.h>

#include <QDir>
#include <QSize>
#include <QResizeEvent>

#include "musicmessageview.h"
#include "durl.h"
#include "stub.h"
#include "addr_pri.h"

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>
#include <QTextCodec>

namespace {
class TestMusicMessageView : public testing::Test
{
public:

    void SetUp() override
    {
        m_url = "file:/usr/share/music/bensound-sunny.mp3";
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

ACCESS_PRIVATE_FUN(MusicMessageView, MediaMeta(const QString &), tagOpenMusicFile);
MediaMeta PrivatetagOpenMusicFile(MusicMessageView * musicView, const QString & path){
    return call_private_fun::MusicMessageViewtagOpenMusicFile(*musicView, path);
}
TEST_F(TestMusicMessageView, can_tag_open_music_file)
{
    EXPECT_TRUE(PrivatetagOpenMusicFile(m_musicMessageView, m_url).title.toStdString().c_str());
    EXPECT_TRUE(PrivatetagOpenMusicFile(m_musicMessageView, m_url).album.toStdString().c_str());
    EXPECT_TRUE(PrivatetagOpenMusicFile(m_musicMessageView, m_url).artist.toStdString().c_str());
}

ACCESS_PRIVATE_FUN(MusicMessageView, void(MediaMeta &, void *), characterEncodingTransform);
void PrivatecharacterEncodingTransform(MusicMessageView * musicView, MediaMeta & meta, void * tag){
    return call_private_fun::MusicMessageViewcharacterEncodingTransform(*musicView, meta, tag);
}
TEST_F(TestMusicMessageView, can_characterEncodingTransform)
{
    MediaMeta meta;
    QUrl url(m_url);
    TagLib::FileRef f(url.toLocalFile().toLocal8Bit());

    TagLib::Tag *tag = f.tag();
    PrivatecharacterEncodingTransform(m_musicMessageView, meta, static_cast<void*>(tag));
    EXPECT_TRUE(meta.title.toStdString().c_str());
    EXPECT_TRUE(meta.album.toStdString().c_str());
    EXPECT_TRUE(meta.artist.toStdString().c_str());
}

ACCESS_PRIVATE_FUN(MusicMessageView, QList<QByteArray>(const QByteArray &), detectEncodings);
QList<QByteArray> PrivateDetectEncodings(MusicMessageView *musicView, const QByteArray & detectByte){
    return call_private_fun::MusicMessageViewdetectEncodings(*musicView, detectByte);
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
            EXPECT_FALSE(PrivateDetectEncodings(m_musicMessageView,detectByte).isEmpty());

            void (*st_ucsdet_setText) (UCharsetDetector * UCD, const char* data, int len, UErrorCode * code) = [](UCharsetDetector*, const char*, int, UErrorCode *code){
                *code = U_ILLEGAL_ARGUMENT_ERROR;
            };

            Stub stub;
            stub.set(ucsdet_setText, st_ucsdet_setText);
            EXPECT_FALSE(PrivateDetectEncodings(m_musicMessageView,detectByte).isEmpty());
        }
    }
}

typedef QMap<QString, QByteArray> mapType;
ACCESS_PRIVATE_FIELD(MusicMessageView,  mapType, localeCodes);
mapType PrivateLocaleCodes(MusicMessageView * musicView){
    return access_private_field::MusicMessageViewlocaleCodes(*musicView);
};
ACCESS_PRIVATE_FUN(MusicMessageView, bool(const QChar &), isChinese);
bool PrivateIsChinese(MusicMessageView *musicView, const QChar & data){
    return call_private_fun::MusicMessageViewisChinese(*musicView, data);
};
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
            auto allDetectCodecs = PrivateDetectEncodings(m_musicMessageView, detectByte);
            auto localeCode = PrivateLocaleCodes(m_musicMessageView).value(QLocale::system().name());

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
                EXPECT_TRUE(PrivateIsChinese(m_musicMessageView, ch));
                if (PrivateIsChinese(m_musicMessageView, ch))
                {
                    detectCodec = "GB18030";
                    break;
                }
            }
        }
    }
}

ACCESS_PRIVATE_FIELD(MusicMessageView,  QMediaPlayer*, m_player);
QMediaPlayer* PrivatePlayer(MusicMessageView * musicView){
    return access_private_field::MusicMessageViewm_player(*musicView);
};
TEST_F(TestMusicMessageView, can_play)
{
    emit PrivatePlayer(m_musicMessageView)->mediaStatusChanged(QMediaPlayer::LoadedMedia);
}

TEST_F(TestMusicMessageView, use_resizeEvent)
{
    m_musicMessageView->setFixedSize(200, 300);
    m_musicMessageView->setFixedSize(400, 600);
}
