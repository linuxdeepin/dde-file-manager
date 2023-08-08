// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "musicmessageview.h"

#include <gtest/gtest.h>

#include <QLabel>
#include <QResizeEvent>
#include <QTextCodec>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>

PREVIEW_USE_NAMESPACE
using namespace TagLib;

class UTTag : Tag
{
public:
    UTTag():Tag(){}
    String title() const { return "UT"; }
    String artist() const { return "UT"; }
    String album() const { return alb; }
    String comment() const { return "UT"; }
    String genre() const { return "UT"; }
    unsigned int year() const { return 1; }
    unsigned int track() const { return 1; }
    void setTitle(const String &s) { Q_UNUSED(s); }
    void setArtist(const String &s) { Q_UNUSED(s); }
    void setAlbum(const String &s) { alb = s; }
    void setComment(const String &s) { Q_UNUSED(s); }
    void setGenre(const String &s) { Q_UNUSED(s); }
    void setYear(unsigned int i) { Q_UNUSED(i); }
    void setTrack(unsigned int i) { Q_UNUSED(i); }

private:
    String alb { "" };
};

TEST(UT_musicMessageView, initUI)
{
    MusicMessageView view;

    EXPECT_TRUE(view.titleLabel);
}

TEST(UT_musicMessageView, updateElidedText)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QFontMetrics::elidedText, [ &isOk ]{
        isOk = true;
        return "UT_TEST";
    });

    MusicMessageView view;
    view.updateElidedText();

    EXPECT_TRUE(isOk);
}

TEST(UT_musicMessageView, mediaStatusChanged)
{
    MusicMessageView view;
    view.mediaStatusChanged(QMediaPlayer::BufferedMedia);

    EXPECT_FALSE(view.fileArtist.isEmpty());
}

TEST(UT_musicMessageView, resizeEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(QWidget::*FuncType)(int, int, int, int);
    stub.set_lamda(static_cast<FuncType>(&QWidget::setContentsMargins), [ &isOk ]{
        isOk = true;
    });

    MusicMessageView view;
    QResizeEvent event(QSize(0, 0), QSize(0, 0));
    view.resizeEvent(&event);

    EXPECT_TRUE(isOk);
}

TEST(UT_musicMessageView, detectEncodings)
{
    MusicMessageView view;
    QList<QByteArray> re = view.detectEncodings(QByteArray("UT_TEST"));

    EXPECT_FALSE(re.isEmpty());
}

TEST(UT_musicMessageView, isChinese)
{
    MusicMessageView view;
    EXPECT_FALSE(view.isChinese(QChar('u')));
}

TEST(UT_musicMessageView, tagOpenMusicFile)
{
    MusicMessageView view;
    MediaMeta data = view.tagOpenMusicFile("/UT_FILE");

    EXPECT_TRUE(data.title.isEmpty());
}

TEST(UT_musicMessageView, characterEncodingTransform_one)
{
    MusicMessageView view;
    MediaMeta meta;
    UTTag tag;
    tag.setAlbum("UT");
    view.characterEncodingTransform(meta, &tag);

    EXPECT_TRUE(meta.album == "UT");
}

TEST(UT_musicMessageView, characterEncodingTransform_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&String::isLatin1, [ &isOk ]{
        isOk = true;
        return false;
    });

    MusicMessageView view;
    MediaMeta meta;
    UTTag tag;
    tag.setAlbum("UT");
    view.characterEncodingTransform(meta, &tag);

    EXPECT_TRUE(isOk);
}

TEST(UT_musicMessageView, characterEncodingTransform_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&MusicMessageView::isChinese, [ &isOk ]{
        isOk = true;
        return true;
    });

    MusicMessageView view;
    MediaMeta meta;
    UTTag tag;
    tag.setAlbum("UT");
    view.characterEncodingTransform(meta, &tag);

    EXPECT_TRUE(isOk);
}

TEST(UT_musicMessageView, characterEncodingTransform_four)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&MusicMessageView::isChinese, []{
        return true;
    });
    typedef  QTextCodec*(*FuncType)(const QByteArray &);
    stub.set_lamda(static_cast<FuncType>(&QTextCodec::codecForName), [ &isOk ]{
        isOk = true;
        return nullptr;
    });

    MusicMessageView view;
    MediaMeta meta;
    UTTag tag;
    tag.setAlbum("UT");
    view.characterEncodingTransform(meta, &tag);

    EXPECT_TRUE(isOk);
}
