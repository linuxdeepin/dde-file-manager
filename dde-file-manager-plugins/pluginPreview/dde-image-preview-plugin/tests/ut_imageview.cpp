/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-25

 */

#include <gtest/gtest.h>

#include "imageview.h"
#include "durl.h"

#include <QImageReader>

namespace  {
    class TestImageView : public testing::Test {
    public:

        void SetUp() override
        {
            QByteArray format;
            m_url = DUrl("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin/tests/123.png");
            m_imageView = new ImageView(QString("test"), format, nullptr);
        }

        void TearDown() override
        {
            delete m_imageView;
            m_imageView = nullptr;
        }

    public:

        ImageView * m_imageView;
        DUrl m_url;
    };
}

TEST_F(TestImageView, can_set_file){
    QByteArray format = QImageReader::imageFormat(m_url.toLocalFile());
    m_imageView->setFile(url.toLocalFile(), format);
    EXPECT_TRUE((!format.isEmpty()));
}

TEST_F(TestImageView, get_source_size){
    QByteArray format = QImageReader::imageFormat(m_url.toLocalFile());
    m_imageView->setFile(url.toLocalFile(), format);
    EXPECT_TRUE(m_imageView->sourceSize().isValid());
}
