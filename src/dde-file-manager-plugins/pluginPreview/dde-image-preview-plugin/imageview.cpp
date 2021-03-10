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

#include "imageview.h"

#include <QUrl>
#include <QImageReader>
#include <QApplication>
#include <QDesktopWidget>
#include <QtMath>
#include <QPainter>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMovie>

#define MIN_SIZE QSize(400, 300)

ImageView::ImageView(const QString &fileName, const QByteArray &format, QWidget *parent)
    : QLabel(parent)
{
    setFile(fileName, format);
    setMinimumSize(MIN_SIZE);
    setAlignment(Qt::AlignCenter);
}

void ImageView::setFile(const QString &fileName, const QByteArray &format)
{
    if (format == QByteArrayLiteral("gif")) {
        if (movie) {
            movie->stop(); // blumia: we need to stop it first before we load a new file
            movie->setFileName(fileName);
        } else {
            movie = new QMovie(fileName, format, this);
        }

        setMovie(movie);
        movie->start();
        m_sourceSize = movie->frameRect().size();

        return;
    } else {
        setMovie(nullptr);
    }

    if (movie) {
        QMovie * tmpMovie = movie;
        movie = nullptr;
        tmpMovie->stop();
        tmpMovie->disconnect();
        tmpMovie->deleteLater();
    }

    QImageReader reader(fileName, format);

    m_sourceSize = reader.size();

    const QSize &dsize = qApp->desktop()->size();
    qreal device_pixel_ratio = this->devicePixelRatioF();

    QPixmap pixmap = QPixmap::fromImageReader(&reader).scaled(QSize(qMin(static_cast<int>(dsize.width() * 0.7 * device_pixel_ratio), m_sourceSize.width()),
                                                                    qMin(static_cast<int>(dsize.height() * 0.8 * device_pixel_ratio), m_sourceSize.height())),
                                                              Qt::KeepAspectRatio, Qt::SmoothTransformation);

    pixmap.setDevicePixelRatio(device_pixel_ratio);

    setPixmap(pixmap);
}

QSize ImageView::sourceSize() const
{
    return m_sourceSize;
}
