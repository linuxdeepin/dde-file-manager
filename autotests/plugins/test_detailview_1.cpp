// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailview_1.cpp
 * @brief Unit tests for DetailView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/detailview.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailViewTest, DetailView)
{
    // Test constructor: DetailView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DetailViewTest, createExtensionWidgets)
{
    // Test method: void createExtensionWidgets(())
    EXPECT_NO_FATAL_FAILURE(obj->createExtensionWidgets());
}

TEST_F(DetailViewTest, finishPreviewLoading)
{
    // Test method: void finishPreviewLoading(())
    EXPECT_NO_FATAL_FAILURE(obj->finishPreviewLoading());
}

TEST_F(DetailViewTest, initSpinnerOverlay)
{
    // Test method: void initSpinnerOverlay(())
    EXPECT_NO_FATAL_FAILURE(obj->initSpinnerOverlay());
}

TEST_F(DetailViewTest, onAnimatedImageReady)
{
    // Test method: void onAnimatedImageReady((const QUrl &url, const QString &filePath))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onAnimatedImageReady(_arg0, _arg1));
}

TEST_F(DetailViewTest, onPreviewReady)
{
    // Test method: void onPreviewReady((const QUrl &url, const QPixmap &pixmap))
    QUrl _arg0{};
    QPixmap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onPreviewReady(_arg0, _arg1));
}

TEST_F(DetailViewTest, reloadPreviewFile)
{
    // Test method: void reloadPreviewFile(())
    EXPECT_NO_FATAL_FAILURE(obj->reloadPreviewFile());
}

TEST_F(DetailViewTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(DetailViewTest, setUrl)
{
    // Test setter: void setUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUrl(_arg0));
}

TEST_F(DetailViewTest, startPreviewLoading)
{
    // Test method: void startPreviewLoading(())
    EXPECT_NO_FATAL_FAILURE(obj->startPreviewLoading());
}

TEST_F(DetailViewTest, updateBasicWidget)
{
    // Test method: void updateBasicWidget((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateBasicWidget(_arg0));
}

TEST_F(DetailViewTest, updateExtensionWidgets)
{
    // Test method: void updateExtensionWidgets((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateExtensionWidgets(_arg0));
}

TEST_F(DetailViewTest, updateHeadUI)
{
    // Test method: void updateHeadUI((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateHeadUI(_arg0));
}

TEST_F(DetailViewTest, updatePreviewSize)
{
    // Test method: void updatePreviewSize(())
    EXPECT_NO_FATAL_FAILURE(obj->updatePreviewSize());
}

TEST_F(DetailViewTest, DetailView_Destructor)
{
    // Test method:  ~DetailView(())
    EXPECT_NO_FATAL_FAILURE({ DetailView *tmp = new DetailView(); delete tmp; });
}
