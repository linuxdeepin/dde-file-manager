// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_imagepreviewworker.cpp
 * @brief Unit tests for ImagePreviewWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/imagepreviewworker.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class ImagePreviewWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ImagePreviewWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ImagePreviewWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ImagePreviewWorkerTest, ImagePreviewWorker)
{
    // Test constructor: ImagePreviewWorker((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ImagePreviewWorkerTest, isImageMimeType)
{
    // Test method: bool isImageMimeType((const QString &mimeType))
    QString _arg0{};
    auto result = obj->isImageMimeType(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ImagePreviewWorkerTest, loadOriginalImage)
{
    // Test method: QPixmap loadOriginalImage((const QString &filePath, const QSize &targetSize))
    QString _arg0{};
    QSize _arg1{};
    auto result = obj->loadOriginalImage(_arg0, _arg1);
    EXPECT_TRUE(result.isNull());

}

TEST_F(ImagePreviewWorkerTest, loadThumbnail)
{
    // Test method: QPixmap loadThumbnail((const QUrl &url, const QSize &targetSize))
    QUrl _arg0{};
    QSize _arg1{};
    auto result = obj->loadThumbnail(_arg0, _arg1);
    EXPECT_TRUE(result.isNull());

}

TEST_F(ImagePreviewWorkerTest, shouldSkipOriginalImageLoad)
{
    // Test method: bool shouldSkipOriginalImageLoad((const QUrl &url, qint64 fileSize))
    QUrl _arg0{};
    auto result = obj->shouldSkipOriginalImageLoad(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(ImagePreviewWorkerTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(ImagePreviewWorkerTest, ImagePreviewWorker_Destructor)
{
    // Test method:  ~ImagePreviewWorker(())
    EXPECT_NO_FATAL_FAILURE({ ImagePreviewWorker *tmp = new ImagePreviewWorker(); delete tmp; });
}
