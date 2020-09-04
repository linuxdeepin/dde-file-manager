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

#include <QByteArray>

DFM_BEGIN_NAMESPACE

namespace  {
    class TestImagePerview : public testing::Test {
    public:

        void SetUp() override
        {
            m_imagePreview = new ImagePreview();
            m_imagePreview->initialize(nullptr, nullptr);
        }

        void TearDown() override
        {
            delete m_imagePreview;
            m_imagePreview = nullptr;
        }

    public:

        ImagePreview * m_imagePreview;
    };
}

TEST_F(TestImagePerview, can_preview){
    QByteArray format;
    QUrl url(QString("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin/tests/123.png"));
    EXPECT_TRUE(m_imagePreview->canPreview(url, &format));
    EXPECT_TRUE((format != nullptr));
}

TEST_F(TestImagePerview, can_set_fileurl){
    DUrl url("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin/tests/123.png");
    EXPECT_TRUE(m_imagePreview->setFileUrl(url));
}

TEST_F(TestImagePerview, get_file_url){
    DUrl url("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin/tests/123.png");
    m_imagePreview->setFileUrl(url);
    EXPECT_TRUE(url == m_imagePreview->fileUrl());
}

TEST_F(TestImagePerview, get_title){
    DUrl url("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin/tests/123.png");
    m_imagePreview->setFileUrl(url);
    EXPECT_TRUE((!m_imagePreview->title().isEmpty()));
}

TEST_F(TestImagePerview, get_contentWidget){
    DUrl url("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin/tests/123.png");
    m_imagePreview->setFileUrl(url);
    EXPECT_TRUE(m_imagePreview->contentWidget() != nullptr);
}

DFM_END_NAMESPACE
