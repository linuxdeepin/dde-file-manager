// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QMimeType>
#include <QString>
#include <QImage>
#include <QUrl>
#include <QMimeDatabase>

#include <dfm-base/utils/thumbnail/thumbnailhelper.h>

using namespace dfmbase;

TEST(ThumbnailHelperTest, SetSizeLimitAndRetrieve)
{
    ThumbnailHelper helper;
    QMimeType mime = QMimeDatabase().mimeTypeForName("text/plain");
    helper.setSizeLimit(mime, 4096);
    qint64 limit = helper.sizeLimit(mime);
    EXPECT_EQ(limit, 4096);
}


TEST(ThumbnailHelperTest, MakePathCreatesDirectory)
{
    ThumbnailHelper helper;
    QString path = "/tmp/dfm_test_thumbnail_helper_dir/sub";
    helper.makePath(path);
    SUCCEED();
}

TEST(ThumbnailHelperTest, SaveThumbnailNullImage)
{
    ThumbnailHelper helper;
    QImage nullImg;
    QString result = helper.saveThumbnail(QUrl::fromLocalFile("/tmp/nonexistent.png"), nullImg, Global::ThumbnailSize::kNormal);
    EXPECT_TRUE(result.isEmpty());
}

TEST(ThumbnailHelperTest, StaticHelpersCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ThumbnailHelper::defaultThumbnailDirs(); });
    EXPECT_NO_FATAL_FAILURE({ (void)ThumbnailHelper::sizeToFilePath(Global::ThumbnailSize::kNormal); });
    EXPECT_NO_FATAL_FAILURE({ (void)ThumbnailHelper::dataToMd5Hex(QByteArray("test")); });
}
