// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "musicmessageview.h"
#include "cover.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QResizeEvent>
#include <QTemporaryDir>
#include <QUrl>
#include <QLabel>

#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>

using namespace plugin_filepreview;

class UT_MusicMessageView : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        delete view;
        view = nullptr;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        qApp->processEvents();
        stub.clear();
    }

    QString toUri(const QString &path)
    {
        return QUrl::fromLocalFile(path).toString();
    }

    QByteArray makePng(int w, int h, const QColor &color)
    {
        QImage img(w, h, QImage::Format_ARGB32);
        img.fill(color);
        QBuffer buf;
        img.save(&buf, "PNG");
        return buf.data();
    }

    QString makeMp3(const QString &name, const char *title, const char *artist, const char *album, const QByteArray &coverPng = {})
    {
        const QString &path = dir.filePath(name);
        {
            QFile file(path);
            if (!file.open(QIODevice::WriteOnly))
                return QString();
            QByteArray frame(4 + 417, '\0');
            frame[0] = char(0xFF);
            frame[1] = char(0xFB);
            frame[2] = char(0x90);
            frame[3] = '\0';
            file.write(frame);
        }

        TagLib::MPEG::File tf(path.toLocal8Bit());
        if (!tf.isValid())
            return path;
        TagLib::ID3v2::Tag *tag = tf.ID3v2Tag(true);
        if (title)
            tag->setTitle(TagLib::String(title, TagLib::String::UTF8));
        if (artist)
            tag->setArtist(TagLib::String(artist, TagLib::String::UTF8));
        if (album)
            tag->setAlbum(TagLib::String(album, TagLib::String::UTF8));
        if (!coverPng.isEmpty()) {
            auto *pic = new TagLib::ID3v2::AttachedPictureFrame;
            pic->setMimeType("image/png");
            pic->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
            pic->setPicture(TagLib::ByteVector(coverPng.constData(), coverPng.size()));
            tag->addFrame(pic);
        }
        tf.save();
        return path;
    }

protected:
    MusicMessageView *view { nullptr };
    QTemporaryDir dir;
    stub_ext::StubExt stub;
};

TEST_F(UT_MusicMessageView, Construct_Mp3WithTag_ShowsTagValues)
{
    const QString &path = makeMp3("tagged.mp3", "TestTitle", "TestArtist", "TestAlbum");
    ASSERT_FALSE(path.isEmpty());

    view = new MusicMessageView(toUri(path));
    EXPECT_EQ(view->titleLabel->text(), QString("TestTitle"));
    EXPECT_EQ(view->artistValue->text(), QString("TestArtist"));
    EXPECT_EQ(view->albumValue->text(), QString("TestAlbum"));
}

TEST_F(UT_MusicMessageView, Construct_Mp3WithoutTag_FallsBackToFileInfo)
{
    const QString &path = makeMp3("plain.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());

    view = new MusicMessageView(toUri(path));
    EXPECT_EQ(view->titleLabel->text(), QString("plain"));
    EXPECT_EQ(view->artistValue->text(), MusicMessageView::tr("unknown artist"));
    EXPECT_EQ(view->albumValue->text(), MusicMessageView::tr("unknown album"));
}

TEST_F(UT_MusicMessageView, Construct_NonExistentFile_FallsBackToFileInfo)
{
    view = new MusicMessageView("/nonexistent-dir/nonexistent.mp3");
    EXPECT_EQ(view->titleLabel->text(), QString("nonexistent"));
    EXPECT_EQ(view->artistValue->text(), MusicMessageView::tr("unknown artist"));
    EXPECT_EQ(view->albumValue->text(), MusicMessageView::tr("unknown album"));
}

TEST_F(UT_MusicMessageView, Construct_EmptyUrl_FallsBackToEmptyInfo)
{
    view = new MusicMessageView(QString(""));
    EXPECT_TRUE(view->titleLabel->text().isEmpty());
    EXPECT_EQ(view->artistValue->text(), MusicMessageView::tr("unknown artist"));
}

TEST_F(UT_MusicMessageView, Construct_Mp3WithCoverArt_ShowsCoverPixmap)
{
    const QString &path = makeMp3("covered.mp3", "T", "A", "B", makePng(64, 64, Qt::red));
    ASSERT_FALSE(path.isEmpty());

    view = new MusicMessageView(toUri(path));
    view->imgLabel->resize(240, 240);
    const QImage &shot = view->imgLabel->grab().toImage();
    EXPECT_EQ(shot.pixelColor(shot.width() / 2, shot.height() / 2).rgb(), qRgb(255, 0, 0));
}

TEST_F(UT_MusicMessageView, SetMediaInfo_AfterManualFieldChange_RefreshesLabels)
{
    const QString &path = makeMp3("tagged2.mp3", "TestTitle", "TestArtist", "TestAlbum");
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    view->fileTitle = "ManualTitle";
    view->fileArtist = "ManualArtist";
    view->fileAlbum = "ManualAlbum";
    view->setMediaInfo();

    EXPECT_EQ(view->titleLabel->text(), QString("TestTitle"));
    EXPECT_EQ(view->artistValue->text(), QString("TestArtist"));
    EXPECT_EQ(view->albumValue->text(), QString("TestAlbum"));
}

TEST_F(UT_MusicMessageView, UpdateElidedText_LongTitle_TextElidedWithEllipsis)
{
    const QString &path = makeMp3("longtitle.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    view->viewMargins = 10;
    view->fileTitle = QString(300, 'A');
    view->fileArtist = QString(300, 'B');
    view->fileAlbum = QString(300, 'C');
    view->updateElidedText();

    EXPECT_TRUE(view->titleLabel->text().endsWith(QString::fromUtf8("…")));
    EXPECT_LT(view->titleLabel->text().size(), 300);
    EXPECT_TRUE(view->artistValue->text().endsWith(QString::fromUtf8("…")));
    EXPECT_TRUE(view->albumValue->text().endsWith(QString::fromUtf8("…")));
}

TEST_F(UT_MusicMessageView, UpdateElidedText_ShortTitle_TextKeptIntact)
{
    const QString &path = makeMp3("short.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    view->viewMargins = 10;
    view->fileTitle = "Short";
    view->fileArtist = "Yes";
    view->fileAlbum = "Ok";
    view->updateElidedText();

    EXPECT_EQ(view->titleLabel->text(), QString("Short"));
    EXPECT_EQ(view->artistValue->text(), QString("Yes"));
    EXPECT_EQ(view->albumValue->text(), QString("Ok"));
}

TEST_F(UT_MusicMessageView, ResizeEvent_LargerSize_MarginsCentered)
{
    const QString &path = makeMp3("resize.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    QResizeEvent event(QSize(700, 400), QSize(600, 300));
    QApplication::sendEvent(view, &event);

    EXPECT_EQ(view->contentsMargins().left(), 80);
    EXPECT_EQ(view->contentsMargins().top(), 80);
    EXPECT_EQ(view->contentsMargins().right(), 0);
}

TEST_F(UT_MusicMessageView, ResizeEvent_NarrowSize_MarginsClamped)
{
    const QString &path = makeMp3("narrow.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    QResizeEvent event(QSize(300, 250), QSize(600, 300));
    QApplication::sendEvent(view, &event);

    EXPECT_EQ(view->contentsMargins().left(), 300 - 250 - 240);
}

TEST_F(UT_MusicMessageView, IsChinese_ChineseCharacter_ReturnsTrue)
{
    const QString &path = makeMp3("cn.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    EXPECT_TRUE(view->isChinese(QString::fromUtf8("中")[0]));
    EXPECT_TRUE(view->isChinese(QChar(0x4E00)));
    EXPECT_TRUE(view->isChinese(QChar(0x9FBF)));
}

TEST_F(UT_MusicMessageView, IsChinese_NonChineseCharacter_ReturnsFalse)
{
    const QString &path = makeMp3("cn2.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    EXPECT_FALSE(view->isChinese(QChar('A')));
    EXPECT_FALSE(view->isChinese(QChar(0x4E00 - 1)));
    EXPECT_FALSE(view->isChinese(QChar(0x9FBF + 1)));
}

TEST_F(UT_MusicMessageView, DetectEncodings_EmptyInput_ReturnsLocaleCharset)
{
    const QString &path = makeMp3("det.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    const QList<QByteArray> &charsets = view->detectEncodings(QByteArray());
    EXPECT_GE(charsets.size(), 1);
    EXPECT_FALSE(charsets.first().isEmpty());
}

TEST_F(UT_MusicMessageView, DetectEncodings_Utf8ChineseText_ReturnsCandidates)
{
    const QString &path = makeMp3("det2.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    const QByteArray &raw = QString::fromUtf8("中文编码检测").toUtf8();
    const QList<QByteArray> &charsets = view->detectEncodings(raw);
    EXPECT_GE(charsets.size(), 1);
}

TEST_F(UT_MusicMessageView, TagOpenMusicFile_ValidMp3_ReturnsMeta)
{
    const QString &path = makeMp3("meta.mp3", "MetaTitle", "MetaArtist", "MetaAlbum");
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    MediaMeta meta = view->tagOpenMusicFile(toUri(path));
    EXPECT_EQ(meta.title, QString("MetaTitle"));
    EXPECT_EQ(meta.artist, QString("MetaArtist"));
    EXPECT_EQ(meta.album, QString("MetaAlbum"));
}

TEST_F(UT_MusicMessageView, TagOpenMusicFile_MissingFile_ReturnsEmptyMeta)
{
    const QString &path = makeMp3("meta2.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    MediaMeta meta = view->tagOpenMusicFile("/nonexistent-dir/none.mp3");
    EXPECT_TRUE(meta.title.isEmpty());
    EXPECT_TRUE(meta.artist.isEmpty());
    EXPECT_TRUE(meta.album.isEmpty());
}

TEST_F(UT_MusicMessageView, CharacterEncodingTransform_UnicodeChinese_KeepsTextAndUtf8Codec)
{
    const QString &path = makeMp3("unicode.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    TagLib::FileRef ref(path.toLocal8Bit());
    ASSERT_TRUE(ref.tag() != nullptr);
    ref.tag()->setTitle(TagLib::String("中文标题", TagLib::String::UTF8));
    ref.tag()->setArtist(TagLib::String("中文歌手", TagLib::String::UTF8));

    MediaMeta meta;
    view->characterEncodingTransform(meta, static_cast<void *>(ref.tag()));
    EXPECT_EQ(meta.title, QString::fromUtf8("中文标题"));
    EXPECT_EQ(meta.artist, QString::fromUtf8("中文歌手"));
    EXPECT_EQ(meta.codec, QString("UTF-8"));
}

TEST_F(UT_MusicMessageView, CharacterEncodingTransform_AsciiText_SetsDetectedCodec)
{
    const QString &path = makeMp3("ascii.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    TagLib::FileRef ref(path.toLocal8Bit());
    ASSERT_TRUE(ref.tag() != nullptr);
    ref.tag()->setTitle(TagLib::String("AsciiTitle", TagLib::String::UTF8));
    ref.tag()->setArtist(TagLib::String("AsciiArtist", TagLib::String::UTF8));
    ref.tag()->setAlbum(TagLib::String("AsciiAlbum", TagLib::String::UTF8));

    MediaMeta meta;
    view->characterEncodingTransform(meta, static_cast<void *>(ref.tag()));
    EXPECT_EQ(meta.title, QString("AsciiTitle"));
    EXPECT_EQ(meta.artist, QString("AsciiArtist"));
    EXPECT_EQ(meta.album, QString("AsciiAlbum"));
    EXPECT_FALSE(meta.codec.isEmpty());
}

TEST_F(UT_MusicMessageView, CharacterEncodingTransform_GbkRawBytes_DetectsGbFamilyCodec)
{
    const QString &path = makeMp3("gbk.mp3", nullptr, nullptr, nullptr);
    ASSERT_FALSE(path.isEmpty());
    view = new MusicMessageView(toUri(path));

    TagLib::FileRef ref(path.toLocal8Bit());
    ASSERT_TRUE(ref.tag() != nullptr);
    const char gbkTitle[] = { char(0xD6), char(0xD0), char(0xCE), char(0xC4), '\0' };
    ref.tag()->setTitle(TagLib::String(gbkTitle, TagLib::String::Latin1));
    ref.tag()->setArtist(TagLib::String(gbkTitle, TagLib::String::Latin1));

    MediaMeta meta;
    view->characterEncodingTransform(meta, static_cast<void *>(ref.tag()));
    EXPECT_FALSE(meta.codec.isEmpty());
    EXPECT_TRUE(meta.codec.contains("GB") || meta.codec.contains("Big5") || meta.codec.contains("UTF")
                || meta.codec.contains("EUC") || meta.codec.contains("ASCII"));
}
