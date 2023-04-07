// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageview.h"

#include <dfm-base/utils/windowutils.h>

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
#include <QScreen>

using namespace plugin_filepreview;
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
    const QSize &dsize = DFMBASE_NAMESPACE::WindowUtils::cursorScreen()->geometry().size();
    qreal device_pixel_ratio = this->devicePixelRatioF();

    if (format == QByteArrayLiteral("gif")) {
        if (movie) {
            movie->stop();   // blumia: we need to stop it first before we load a new file
            movie->setFileName(fileName);
        } else {
            movie = new QMovie(fileName, format, this);
        }
        setMovie(movie);
        movie->start();
        sourceImageSize = QSize(qMin(static_cast<int>(dsize.width() * 0.7 * device_pixel_ratio), movie->frameRect().size().width()),
                                qMin(static_cast<int>(dsize.height() * 0.7 * device_pixel_ratio), movie->frameRect().size().height()));
        setFixedSize(sourceImageSize);
        movie->setScaledSize(sourceImageSize);
        return;
    } else {
        setMovie(nullptr);
    }

    if (movie) {
        QMovie *tmpMovie = movie;
        movie = nullptr;
        tmpMovie->stop();
        tmpMovie->disconnect();
        tmpMovie->deleteLater();
    }

    QImageReader reader(fileName, format);
    sourceImageSize = reader.size();
    QPixmap pixmap = QPixmap::fromImageReader(&reader).scaled(QSize(qMin(static_cast<int>(dsize.width() * 0.7 * device_pixel_ratio), sourceImageSize.width()),
                                                                    qMin(static_cast<int>(dsize.height() * 0.7 * device_pixel_ratio), sourceImageSize.height())),
                                                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(device_pixel_ratio);
    setPixmap(pixmap);
}

QSize ImageView::sourceSize() const
{
    return sourceImageSize;
}
