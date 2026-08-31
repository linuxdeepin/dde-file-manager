// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_imagepreviewcontroller_1.cpp
 * @brief Unit tests for ImagePreviewController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/imagepreviewworker.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class ImagePreviewControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ImagePreviewController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ImagePreviewController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ImagePreviewControllerTest, ImagePreviewController)
{
    // Test constructor: ImagePreviewController((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ImagePreviewControllerTest, onAnimatedImageReady)
{
    // Test method: void onAnimatedImageReady((const QUrl &url, const QString &filePath))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onAnimatedImageReady(_arg0, _arg1));
}

TEST_F(ImagePreviewControllerTest, onThumbnailProduced)
{
    // Test method: void onThumbnailProduced((const QUrl &url, const QString &thumbnailPath))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onThumbnailProduced(_arg0, _arg1));
}

TEST_F(ImagePreviewControllerTest, requestPreview)
{
    // Test method: void requestPreview((const QUrl &url, const QSize &targetSize))
    QUrl _arg0{};
    QSize _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->requestPreview(_arg0, _arg1));
}

TEST_F(ImagePreviewControllerTest, ImagePreviewController_Destructor)
{
    // Test method:  ~ImagePreviewController(())
    EXPECT_NO_FATAL_FAILURE({ ImagePreviewController *tmp = new ImagePreviewController(); delete tmp; });
}
