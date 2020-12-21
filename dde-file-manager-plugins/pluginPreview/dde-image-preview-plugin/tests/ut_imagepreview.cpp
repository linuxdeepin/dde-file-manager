/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-24

 */

#include <gtest/gtest.h>

#include "imagepreview.h"

#include "stub.h"

#include <QByteArray>
#include <QBuffer>

DFM_USE_NAMESPACE

namespace  {
    class TestImagePerview : public testing::Test {
    public:

        void SetUp() override
        {
            m_imagePreview = new ImagePreview();
            m_imagePreview->initialize(nullptr, nullptr);
            QImage image(600, 800, QImage::Format_ARGB32);
            image.fill(1);
            image.save(QString("./123.png"), "PNG");
            m_url =  DUrl(QString("file:123.png"));
            qDebug() << m_url;
        }

        void TearDown() override
        {
            delete m_imagePreview;
            m_imagePreview = nullptr;
            QFile image(QString("./123.png"));
            image.remove();
        }

    public:

        ImagePreview * m_imagePreview;
        DUrl m_url;
    };
}

TEST_F(TestImagePerview, can_preview){
    QByteArray format;

    EXPECT_TRUE(m_imagePreview->canPreview(m_url, &format));
    qDebug() << format;
    EXPECT_FALSE(format.isEmpty());

    bool (*st_isEmpty)() = [](){
        return true;
    };

    format.clear();
    Stub stub;
    stub.set(ADDR(QByteArray,isEmpty), st_isEmpty);
    EXPECT_TRUE(m_imagePreview->canPreview(m_url, &format));
    EXPECT_FALSE(format.isEmpty());
}

TEST_F(TestImagePerview, can_set_fileurl){
    EXPECT_TRUE(m_imagePreview->setFileUrl(m_url));

    bool (*st_canPreview)(const QUrl &url, QByteArray *format) = [](const QUrl &url, QByteArray *format){
        return false;
    };
    Stub stub1;
    stub1.set(&ImagePreview::canPreview, st_canPreview);
    EXPECT_TRUE(m_imagePreview->setFileUrl(m_url));

    bool (*st_isLocalFile)() = [](){
        return false;
    };

    Stub stub;
    stub.set(&DUrl::isLocalFile, st_isLocalFile);
    EXPECT_TRUE(m_imagePreview->setFileUrl(m_url));
}

TEST_F(TestImagePerview, get_file_url){
    m_imagePreview->setFileUrl(m_url);
    EXPECT_TRUE(m_url == m_imagePreview->fileUrl());
}

TEST_F(TestImagePerview, get_title){
    m_imagePreview->setFileUrl(m_url);
    EXPECT_TRUE((!m_imagePreview->title().isEmpty()));
}

TEST_F(TestImagePerview, get_contentWidget){
    m_imagePreview->setFileUrl(m_url);
    EXPECT_TRUE(m_imagePreview->contentWidget() != nullptr);
}

TEST_F(TestImagePerview, used_copyFile){
    EXPECT_NO_FATAL_FAILURE(m_imagePreview->copyFile());
}
