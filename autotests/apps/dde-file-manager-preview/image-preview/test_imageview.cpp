// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageview.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QMovie>
#include <QPixmap>
#include <QTemporaryDir>
#include <QWidget>

using namespace plugin_filepreview;

class UT_ImageView : public testing::Test
{
protected:
    void SetUp() override {}

    void TearDown() override {}

    QString makeImageFile(const QString &name, int width, int height, const char *format)
    {
        QImage image(width, height, QImage::Format_ARGB32);
        image.fill(Qt::blue);
        const QString &path = dir.filePath(name);
        if (!image.save(path, format))
            return QString();
        return path;
    }

    QString makeGifFile(const QString &name)
    {
        const QByteArray &gifData = QByteArray::fromBase64(
                "R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7");
        const QString &path = dir.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
            return QString();
        file.write(gifData);
        file.close();
        return path;
    }

protected:
    QTemporaryDir dir;
};

TEST_F(UT_ImageView, Construct_WithValidPng_LoadsPixmapAndAppliesDefaults)
{
    const QString &path = makeImageFile("construct.png", 64, 48, "PNG");
    ASSERT_FALSE(path.isEmpty());

    ImageView view(path, QByteArray("png"));

    EXPECT_EQ(view.sourceSize(), QSize(64, 48));

    EXPECT_FALSE(view.pixmap().isNull());
    EXPECT_GE(view.pixmap().devicePixelRatio(), 1.0);
    EXPECT_EQ(view.minimumSize(), QSize(400, 300));
    EXPECT_EQ(view.maximumSize(), QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    EXPECT_EQ(static_cast<int>(view.alignment()), static_cast<int>(Qt::AlignCenter));
}

TEST_F(UT_ImageView, Construct_WithNonExistentFile_ShowsEmptyPixmap)
{
    ImageView view(dir.filePath("missing.png"), QByteArray("png"));

    EXPECT_TRUE(view.pixmap().isNull());
    EXPECT_FALSE(view.sourceSize().isValid());
}

TEST_F(UT_ImageView, SetFile_AnotherValidPng_UpdatesPixmapAndSourceSize)
{
    const QString &firstPath = makeImageFile("first.png", 64, 48, "PNG");
    const QString &secondPath = makeImageFile("second.png", 100, 50, "PNG");
    ASSERT_FALSE(firstPath.isEmpty());
    ASSERT_FALSE(secondPath.isEmpty());

    ImageView view(firstPath, QByteArray("png"));
    view.setFile(secondPath, QByteArray("png"));

    EXPECT_EQ(view.sourceSize(), QSize(100, 50));

    EXPECT_FALSE(view.pixmap().isNull());
}

TEST_F(UT_ImageView, SetFile_LargeImage_KeepsOriginalSourceSize)
{
    const QString &path = makeImageFile("large.png", 3000, 2000, "PNG");
    ASSERT_FALSE(path.isEmpty());

    ImageView view(path, QByteArray("png"));

    EXPECT_EQ(view.sourceSize(), QSize(3000, 2000));

    EXPECT_FALSE(view.pixmap().isNull());
}

TEST_F(UT_ImageView, SetFile_EmptyFormat_DetectsFormatFromContent)
{
    const QString &path = makeImageFile("noformat.png", 40, 30, "PNG");
    ASSERT_FALSE(path.isEmpty());

    ImageView view(path, QByteArray());

    EXPECT_EQ(view.sourceSize(), QSize(40, 30));

    EXPECT_FALSE(view.pixmap().isNull());
}

TEST_F(UT_ImageView, SetFile_GifFile_CreatesAndStartsMovie)
{
    const QString &path = makeGifFile("animated.gif");
    ASSERT_FALSE(path.isEmpty());

    ImageView view(path, QByteArray("gif"));

    QMovie *movie = view.movie;
    ASSERT_NE(movie, nullptr);
    EXPECT_EQ(movie->fileName(), path);
    EXPECT_EQ(movie->state(), QMovie::Running);
    EXPECT_EQ(view.sourceSize(), QSize(1, 1));
}

TEST_F(UT_ImageView, SetFile_SecondGif_ReusesSameMovieObject)
{
    const QString &firstPath = makeGifFile("first.gif");
    const QString &secondPath = makeGifFile("second.gif");
    ASSERT_FALSE(firstPath.isEmpty());
    ASSERT_FALSE(secondPath.isEmpty());

    ImageView view(firstPath, QByteArray("gif"));
    QMovie *firstMovie = view.movie;
    ASSERT_NE(firstMovie, nullptr);

    view.setFile(secondPath, QByteArray("gif"));

    EXPECT_EQ(view.movie, firstMovie);
    EXPECT_EQ(view.movie->fileName(), secondPath);
}

TEST_F(UT_ImageView, SetFile_GifThenStaticImage_StopsAndClearsMovie)
{
    const QString &gifPath = makeGifFile("clip.gif");
    const QString &pngPath = makeImageFile("static.png", 50, 40, "PNG");
    ASSERT_FALSE(gifPath.isEmpty());
    ASSERT_FALSE(pngPath.isEmpty());

    ImageView view(gifPath, QByteArray("gif"));
    ASSERT_NE(view.movie, nullptr);

    view.setFile(pngPath, QByteArray("png"));
    qApp->processEvents();

    EXPECT_EQ(view.movie, nullptr);

    EXPECT_FALSE(view.pixmap().isNull());
    EXPECT_EQ(view.sourceSize(), QSize(50, 40));
    EXPECT_EQ(view.minimumSize(), QSize(400, 300));
    EXPECT_EQ(view.maximumSize(), QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
}

TEST_F(UT_ImageView, SetFile_GifOnConstructedView_AppliesFixedShowSize)
{
    const QString &pngPath = makeImageFile("initial.png", 64, 48, "PNG");
    const QString &gifPath = makeGifFile("fixed.gif");
    ASSERT_FALSE(pngPath.isEmpty());
    ASSERT_FALSE(gifPath.isEmpty());

    ImageView view(pngPath, QByteArray("png"));
    view.setFile(gifPath, QByteArray("gif"));

    EXPECT_NE(view.movie, nullptr);
    EXPECT_EQ(view.minimumSize(), QSize(1, 1));
    EXPECT_EQ(view.maximumSize(), QSize(1, 1));
    EXPECT_EQ(view.movie->scaledSize(), QSize(1, 1));
}

TEST_F(UT_ImageView, SetFile_NonExistentFile_ClearsPixmap)
{
    const QString &path = makeImageFile("loaded.png", 64, 48, "PNG");
    ASSERT_FALSE(path.isEmpty());

    ImageView view(path, QByteArray("png"));


    view.setFile(dir.filePath("vanished.png"), QByteArray("png"));

    EXPECT_TRUE(view.pixmap().isNull());
    EXPECT_FALSE(view.sourceSize().isValid());
}

TEST_F(UT_ImageView, SourceSize_AfterLoad_MatchesImageDimensions)
{
    const QString &path = makeImageFile("dimensions.png", 123, 45, "PNG");
    ASSERT_FALSE(path.isEmpty());

    ImageView view(path, QByteArray("png"));

    EXPECT_EQ(view.sourceSize().width(), 123);
    EXPECT_EQ(view.sourceSize().height(), 45);
}
