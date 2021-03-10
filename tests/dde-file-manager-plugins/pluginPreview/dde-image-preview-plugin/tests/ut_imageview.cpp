/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>

#include "imageview.h"
#include "durl.h"

#include <QImageReader>
#include <QFile>

class TestImageView : public testing::Test {
public:

    void SetUp() override
    {
        QByteArray format;
        QImage image(600, 800, QImage::Format_ARGB32);
        image.fill(1);
        image.save(QString("./123.png"), "PNG");
        m_url = DUrl("file:123.png");
        m_imageView = new ImageView(QString("test"), format, nullptr);
    }

    void TearDown() override
    {
        delete m_imageView;
        m_imageView = nullptr;
        QFile image(QString("./123.png"));
        image.remove();
    }

public:

    ImageView * m_imageView;
    DUrl m_url;
};

TEST_F(TestImageView, can_set_file){
    QByteArray format = QImageReader::imageFormat(m_url.toLocalFile());
    m_imageView->setFile(m_url.toLocalFile(), format);
    EXPECT_FALSE(format.isEmpty());

    format = QByteArray("gif");
    m_imageView->setFile(m_url.toLocalFile(), format);
    EXPECT_FALSE(format.isEmpty());

    format = QByteArray("png");
    m_imageView->setFile(m_url.toLocalFile(), format);
    EXPECT_FALSE(format.isEmpty());
}

TEST_F(TestImageView, get_source_size){
    QByteArray format = QImageReader::imageFormat(m_url.toLocalFile());
    m_imageView->setFile(m_url.toLocalFile(), format);
    EXPECT_TRUE(m_imageView->sourceSize().isValid());
}
