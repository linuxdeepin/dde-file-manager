// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_imagepreviewwidget_1.cpp
 * @brief Unit tests for ImagePreviewWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/imagepreviewwidget.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class ImagePreviewWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ImagePreviewWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ImagePreviewWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ImagePreviewWidgetTest, ImagePreviewWidget)
{
    // Test constructor: ImagePreviewWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ImagePreviewWidgetTest, isAnimatedMimeType)
{
    // Test method: bool isAnimatedMimeType((const QString &mimeType))
    QString _arg0{};
    auto result = obj->isAnimatedMimeType(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ImagePreviewWidgetTest, maximumPreviewSize)
{
    // Test getter: QSize maximumPreviewSize()
    auto result = obj->maximumPreviewSize();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ImagePreviewWidgetTest, onMovieFrameChanged)
{
    // Test method: void onMovieFrameChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onMovieFrameChanged());
}

TEST_F(ImagePreviewWidgetTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(ImagePreviewWidgetTest, setAnimatedImage)
{
    // Test setter: void setAnimatedImage((const QString &filePath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setAnimatedImage(_arg0));
}

TEST_F(ImagePreviewWidgetTest, setPixmap)
{
    // Test setter: void setPixmap((const QPixmap &pixmap))
    QPixmap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPixmap(_arg0));
}

TEST_F(ImagePreviewWidgetTest, sizeHint)
{
    // Test getter: QSize sizeHint()
    auto result = obj->sizeHint();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ImagePreviewWidgetTest, stopAnimatedImage)
{
    // Test method: void stopAnimatedImage(())
    EXPECT_NO_FATAL_FAILURE(obj->stopAnimatedImage());
}

TEST_F(ImagePreviewWidgetTest, ImagePreviewWidget_Destructor)
{
    // Test method:  ~ImagePreviewWidget(())
    EXPECT_NO_FATAL_FAILURE({ ImagePreviewWidget *tmp = new ImagePreviewWidget(); delete tmp; });
}
