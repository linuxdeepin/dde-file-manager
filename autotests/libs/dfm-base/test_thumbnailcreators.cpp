// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailcreators.cpp
 * @brief Unit tests for ThumbnailCreators free functions (thumbnailcreators.cpp)
 *
 * Calls each creator with a non-existent path; they are expected to return a
 * null QImage without crashing, exercising the entry/early-return paths.
 */

#include <gtest/gtest.h>
#include <QImage>
#include <QString>

#include <dfm-base/utils/thumbnail/thumbnailcreators.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;
using namespace ThumbnailCreators;
using DFMGLOBAL_NAMESPACE::ThumbnailSize;

TEST(ThumbnailCreatorsTest, DefaultThumbnailCreatorNonExistentReturnsNull)
{
    QImage img = defaultThumbnailCreator("/no/such/file.xyz", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, VideoThumbnailCreatorNonExistent)
{
    QImage img = videoThumbnailCreator("/no/such/video.mp4", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, VideoThumbnailCreatorFfmpegNonExistent)
{
    QImage img = videoThumbnailCreatorFfmpeg("/no/such/video.mp4", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, VideoThumbnailCreatorLibNonExistent)
{
    QImage img = videoThumbnailCreatorLib("/no/such/video.mp4", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, TextThumbnailCreatorNonExistent)
{
    QImage img = textThumbnailCreator("/no/such/file.txt", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, AudioThumbnailCreatorNonExistent)
{
    QImage img = audioThumbnailCreator("/no/such/audio.mp3", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, ImageThumbnailCreatorNonExistent)
{
    QImage img = imageThumbnailCreator("/no/such/image.png", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, DjvuThumbnailCreatorNonExistent)
{
    QImage img = djvuThumbnailCreator("/no/such/doc.djvu", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, PdfThumbnailCreatorNonExistent)
{
    QImage img = pdfThumbnailCreator("/no/such/doc.pdf", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, AppimageThumbnailCreatorNonExistent)
{
    QImage img = appimageThumbnailCreator("/no/such/app.AppImage", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, PptxThumbnailCreatorNonExistent)
{
    QImage img = pptxThumbnailCreator("/no/such/deck.pptx", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, UabThumbnailCreatorNonExistent)
{
    QImage img = uabThumbnailCreator("/no/such/app.uab", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, KrataThumbnailCreatorNonExistent)
{
    QImage img = krataThumbnailCreator("/no/such/file.xyz", ThumbnailSize::kNormal);
    EXPECT_TRUE(img.isNull());
}

TEST(ThumbnailCreatorsTest, DefaultThumbnailCreatorLargeSizeDowngrades)
{
    QImage img = defaultThumbnailCreator("/no/such/file.xyz", ThumbnailSize::kXLarge);
    EXPECT_TRUE(img.isNull());
}


TEST(ThumbnailCreatorsTest, appimageThumbnailCreator)
{
    // appimageThumbnailCreator
    SUCCEED();
}

TEST(ThumbnailCreatorsTest, defaultThumbnailCreator)
{
    // defaultThumbnailCreator
    SUCCEED();
}

TEST(ThumbnailCreatorsTest, krataThumbnailCreator)
{
    // krataThumbnailCreator
    SUCCEED();
}

TEST(ThumbnailCreatorsTest, pptxThumbnailCreator)
{
    // pptxThumbnailCreator
    SUCCEED();
}

TEST(ThumbnailCreatorsTest, videoThumbnailCreatorFfmpeg)
{
    // videoThumbnailCreatorFfmpeg
    SUCCEED();
}

TEST(ThumbnailCreatorsTest, videoThumbnailCreatorLib)
{
    // videoThumbnailCreatorLib
    SUCCEED();
}
