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
#include <QFile>
#include <QTemporaryDir>

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

// ============================================================
// PMS sev-2 regression cluster: thumbnailcreators.cpp (work-order batch 3)
// ============================================================

// PMS:337795 双击含二进制未签名文件夹提示安全认证：pdfThumbnailCreator 对
// 目录/二进制/不存在路径的 poppler 解析失败必须返回空图，不再崩溃
TEST(ThumbnailCreatorsTest, BUG337795_PdfThumbnailCreatorInvalidInputNullImage)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString dirPath = dir.path();
    const QString binPath = dirPath + "/not_a_pdf_337795.bin";
    {
        QFile f(binPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("\x7fELF\x02\x01\x01\x00 ut binary payload");
    }

    QImage img;
    EXPECT_NO_FATAL_FAILURE({ img = ThumbnailCreators::pdfThumbnailCreator(dirPath, ThumbnailSize::kNormal); });
    EXPECT_TRUE(img.isNull());

    EXPECT_NO_FATAL_FAILURE({ img = ThumbnailCreators::pdfThumbnailCreator(binPath, ThumbnailSize::kNormal); });
    EXPECT_TRUE(img.isNull());

    EXPECT_NO_FATAL_FAILURE({ img = ThumbnailCreators::pdfThumbnailCreator(dirPath + "/missing_337795.pdf", ThumbnailSize::kNormal); });
    EXPECT_TRUE(img.isNull());
}
