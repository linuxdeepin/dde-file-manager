// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QResizeEvent>
#include <QPixmap>

#include "stubext.h"

#include "views/detailview.h"

using namespace dfmplugin_detailspace;

class DetailViewTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        view = new DetailView();
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    DetailView *view = nullptr;
};

// --- construction (covers initInfoUI) ---

TEST_F(DetailViewTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

// --- setUrl with empty url (early return, m_currentUrl is empty) ---

TEST_F(DetailViewTest, SetUrl_EmptyUrl_EarlyReturn)
{
    EXPECT_NO_FATAL_FAILURE(view->setUrl(QUrl()));
}

// --- setUrl with a real file url (covers updateHeadUI, updateBasicWidget, updateExtensionWidgets) ---

TEST_F(DetailViewTest, SetUrl_FileUrl_UpdatesView)
{
    QUrl url("file:///tmp");
    EXPECT_NO_FATAL_FAILURE(view->setUrl(url));
}

TEST_F(DetailViewTest, SetUrl_SameUrlTwice_EarlyReturnSecond)
{
    QUrl url("file:///tmp");
    view->setUrl(url);
    EXPECT_NO_FATAL_FAILURE(view->setUrl(url));
}

TEST_F(DetailViewTest, SetUrl_DifferentUrls_UpdatesEach)
{
    view->setUrl(QUrl("file:///tmp"));
    EXPECT_NO_FATAL_FAILURE(view->setUrl(QUrl("file:///")));
}

// --- resizeEvent ---

TEST_F(DetailViewTest, ResizeEvent_NoCrash)
{
    view->resize(400, 600);
    QResizeEvent event(QSize(400, 600), QSize(200, 300));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(view, &event));
}

// --- onPreviewReady with a pixmap ---

TEST_F(DetailViewTest, OnPreviewReady_MatchingUrl_UpdatesPreview)
{
    view->setUrl(QUrl("file:///tmp"));
    QPixmap pix(64, 64);
    pix.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(view->onPreviewReady(QUrl("file:///tmp"), pix));
}

TEST_F(DetailViewTest, OnPreviewReady_NonMatchingUrl_Ignored)
{
    view->setUrl(QUrl("file:///tmp"));
    QPixmap pix(64, 64);
    pix.fill(Qt::blue);
    EXPECT_NO_FATAL_FAILURE(view->onPreviewReady(QUrl("file:///nonexistent"), pix));
}

// --- onAnimatedImageReady ---

TEST_F(DetailViewTest, OnAnimatedImageReady_MatchingUrl_NoCrash)
{
    view->setUrl(QUrl("file:///tmp"));
    EXPECT_NO_FATAL_FAILURE(view->onAnimatedImageReady(QUrl("file:///tmp"), "/tmp/test.gif"));
}

TEST_F(DetailViewTest, OnAnimatedImageReady_NonMatchingUrl_Ignored)
{
    view->setUrl(QUrl("file:///tmp"));
    EXPECT_NO_FATAL_FAILURE(view->onAnimatedImageReady(QUrl("file:///nonexistent"), "/tmp/test.gif"));
}
